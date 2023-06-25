#include <sysexits.h>
#include <signal.h>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <ranges>

#include "cxxopts.hpp"
#include "fmt/chrono.h"
#include "fmt/core.h"
#include "httpclientconnector.hpp"
#include "jsonrpccxx/client.hpp"
#include "messages.hpp"
#include "messages_fmt.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "string_trim.hpp"
#include "inotify-cpp/NotifierBuilder.h"
#include "lambda_signal.hpp"
#include "device.hpp"

namespace fs = std::filesystem;

using namespace std::string_view_literals;
using namespace std::chrono_literals;

using std::string;
using std::vector;
using std::chrono::seconds;
using std::chrono::minutes;
using cxxopts::ParseResult;
using jsonrpccxx::version;
using jsonrpccxx::JsonRpcClient;
using inotify::BuildNotifier;
using inotify::NotifierBuilder;

using level_enum = spdlog::level::level_enum;
using json       = nlohmann::json;
using JsonObject = std::map<string, json>;

using namespace messages;

constexpr auto DEFAULT_VTOKEN =
    "KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf"sv;

struct Args {
    level_enum     log_level;
    string         host;
    int            port;
    string         base_path;
    string         verify_token;
    bool           detailed;
    bool           system_info;
    bool           system_status;
    bool           connection_state;
    bool           sms_storage_state;
    bool           sms_contact_list;
    int            sms_content_list;
    size_t         page;
    vector<string> send_sms;
    string         content;
    int            delete_sms;
    fs::path       watch;
    fs::path       move_to;
    seconds        time_out;
    seconds        keep_alive;
    minutes        reprocess;
};
std::unique_ptr<Args> parse(int argc, const char *argv[]);
Device DefaultDevice(Args& args);

void process_file(
    Device& client, const fs::path& file, const fs::path& move_to);

int main(int argc, const char *argv[]) {
    spdlog::flush_on(spdlog::level::err);
    spdlog::flush_on(spdlog::level::critical);

    std::unique_ptr<Args> args;
    try {
        spdlog::debug("trying parse arguments ...");
        args = parse(argc, argv);
    } catch (const std::exception &e) {
        spdlog::error("argument parse error: {}", e.what());
        fmt::print(stderr, "see usage: asmsd --help\n");
        std::exit(EX_USAGE);
    }

    spdlog::debug("asmsd - Alcatel (dongle) SMS Daemon");

    if (spdlog::get_level() != args->log_level) {
        spdlog::set_level(args->log_level);
        spdlog::info("log levelt set to: {}",
                     spdlog::level::to_string_view(spdlog::get_level()));
    }

    try {
        spdlog::debug("using host: {}, port: {}", args->host,
                      args->port);

        auto device = DefaultDevice(*args);

        spdlog::debug("wating for device to be alive...");
        device.wait_alive();

        bool specified{};

        if (args->system_info) {
            const auto& sys_inf = device.system_info();
            if (args->detailed)
                fmt::print("{:d}\n", sys_inf);
            else
                fmt::print("{}\n", sys_inf);
            specified = true;
        }

        if (args->system_status) {
            const auto& sys_status = device.system_status();
            if (args->detailed)
                fmt::print("{:d}\n", sys_status);
            else
                fmt::print("{}\n", sys_status);
            specified = true;
        }

        if (args->connection_state) {
            const auto& con_state = device.connection_state();
            if (args->detailed)
                fmt::print("{:d}\n", con_state);
            else
                fmt::print("{}\n", con_state);
            specified = true;
        }

        if (args->sms_storage_state) {
            const auto& sms_state = device.sms_storage_state();
            fmt::print("{}\n", sms_state);
            specified = true;
        }

        if (args->sms_contact_list) {
            const auto& sms_clist = device.sms_contacts(args->page);
            fmt::print("{}\n", sms_clist);
            specified = true;
        }

        if (args->sms_content_list > 0) {
            if (args->delete_sms > 0)
                device.delete_sms(args->sms_content_list, args->delete_sms);
            else {
                const auto& sms_clist = device.sms_contents(
                        args->sms_content_list, args->page);
                fmt::print("{}\n", sms_clist);
            }
            specified = true;
        }

        if (!args->send_sms.empty()) {
            if (args->content.empty()) {
                fmt::print(
                    stderr,
                    "--content is not provided, see usage: asmsd --help\n");
                std::exit(EX_USAGE);
            }

            const auto res = device.send_sms(
                std::move(args->send_sms), 
                std::move(args->content));

            spdlog::debug("SMS send status: {}", res.send_status_sv());

            specified = true;
        }

        if (args->delete_sms > 0 && args->sms_content_list <= 0) {
            device.delete_sms(args->delete_sms);
            specified = true;
        }

        if (!args->watch.empty()) {
            const auto watch_string = args->watch.string();
            if(!fs::is_directory(args->watch)) {
                spdlog::error("watching path: {} is not a directory", watch_string);
                std::exit(EX_USAGE);
            }

            if (args->reprocess > 0min) {
                const auto since = std::chrono::file_clock::now() - args->reprocess;
                spdlog::info("reprocessing files since {:%H:%M:%S}",
                             std::chrono::file_clock::to_sys(since));

                try {
                    for (const auto& dir_entry : fs::directory_iterator{args->watch}) {
                        try {
                            if (dir_entry.is_regular_file() && dir_entry.last_write_time() >= since) {
                                spdlog::debug("reprocessing file: {}", dir_entry.path().string());
                                process_file(device, dir_entry.path(), args->move_to);
                            }
                        } catch (const std::exception &e) {
                            spdlog::error("unable to get director entry: {}", e.what());
                        }
                    }
                } catch (const std::exception &e) {
                    spdlog::error("unable to iterate in directory entries: {}", e.what());
                }
            }

            spdlog::debug("start watching directory for file creation: {}", watch_string);
            auto notifier = BuildNotifier()
                .watchPathRecursively(args->watch)
                .onEvents(
                    {inotify::Event::create, inotify::Event::moved_to}, 
                    [&device, &mv_to = args->move_to](inotify::Notification notif){
                    if (fs::is_regular_file(notif.path)) {
                        spdlog::debug("new file created at {}", notif.path.string());
                        process_file(device, notif.path, mv_to);
                    }
                });
            
            spdlog::debug("registring stop signal: press ^C to stop watching...");
            SignalScope<SIGINT> sigint([&device](int sig){
                spdlog::debug("got signal {}, stopping keepalive...", sig);
                device.stop_keepalive();
            });

            spdlog::debug("start running notifier on watch thread...");
            std::thread watch_thread([&notifier](){ 
                notifier.run();
                spdlog::debug("notifier stoped running...");
            });

            spdlog::debug("start running keepalive thread...");
            std::thread keepalive_thread(
                [&notifier, &device, &path = args->watch](){
                device.run_keepalive([&](Device& d){
                    spdlog::error("device went offline... unwatch directory");
                    notifier.unwatchFile(path);
                    spdlog::info("waiting for device to be alive...");
                    d.wait_alive();
                    spdlog::info("device came back online, start watching again...");
                    notifier.watchFile(path);
                    return d.is_running();
                });
                spdlog::debug("keepalive stoped running...");
            });
            
            spdlog::debug("wating for keepalive thread to stop...");
            keepalive_thread.join();
            spdlog::info("keepalive thread stoped");

            spdlog::debug("stoping notifier...");
            notifier.stop();
            spdlog::debug("waiting for watch thread to stop...");
            watch_thread.join();
            spdlog::info("watch thread stoped");

            specified = true;
        }

        if (!specified)
            fmt::print(stderr,
                       "no command specified, see usage: asmsd --help\n");

    } catch (const std::exception &e) {
        spdlog::critical("json rpc request error: {}", e.what());
        std::exit(EX_SOFTWARE);
    }

    args.reset();

    std::exit(EXIT_SUCCESS);
}

void process_file(
    Device&  client, 
    const fs::path& file, 
    const fs::path& move_to
    ) {
    const auto path_string = file.string();

    std::ifstream in_file{file};
    if (!in_file) {
        spdlog::error("unable to open file: {}", path_string);
        return;
    }

    std::stringstream content{};
    string            phone_number{};
    try {
        string line{};
        while (!(line.starts_with("To") || line.starts_with("to"))) {
            line.clear();
            if (!std::getline(in_file, line)) break;
        }
        if (line.empty()) {
            spdlog::error("no To field specified in file");
            return;
        }
        if (const auto i = line.find_last_of(':'); i > 0) {
            if (i + 1 < line.size()) {
                phone_number = line.substr(i+1);
                trim(phone_number);
            } else {
                spdlog::error("no value in to field (empty)");
                return;
            }
        } else {
            spdlog::error("no separator ':' specified in To field");
            return;
        }
        
        line.clear();
        while (std::getline(in_file, line) && !line.empty())
            line.clear();

        content << in_file.rdbuf();
    } catch (const std::exception &e) {
        spdlog::error("unable to read in file {} content: {}",
            path_string, e.what());
        return;
    }

    spdlog::debug("sending SMS to: {}", phone_number);

    try {
        client.send_sms({std::move(phone_number)}, content.str());
    } catch (const std::exception &e) {
        spdlog::error("unable to send sms: {}", e.what());
        return;
    }

    if (!move_to.empty()) {
        spdlog::debug("moving file {} to {}", 
            file.filename().string(),
            move_to.string());
        try {
            fs::rename(file, move_to/file.filename());
        } catch (const std::exception &e) {
            spdlog::error("unable to move file: {}", e.what());
        }
    }
}

std::unique_ptr<Args> parse(int argc, const char *argv[]) {
    using cxxopts::Options;
    using cxxopts::value;

    auto options_allocated =
        std::make_unique<Options>("asmsd", "ASMSD - Alcatel SMS Daemon");
    auto &opts = *options_allocated;

    auto   args = std::make_unique<Args>();
    string log_level{};
    long   time_out{};
    long   keep_alive{};
    long   reprocess{};

    opts.set_width(90).add_options()  //
        ("l,log-level", "log level", value(log_level)->default_value("info"))(
            "h,host", "hostname or ip address of the device",
            value(args->host)->default_value("192.168.1.1"))(
            "p,port", "target port number of the http connection",
            value(args->port)->default_value("80"))(
            "b,base-path", "base path of the json rpc endpoint",
            value(args->base_path)->default_value("/jrd/webapi"))(
            "v,verify-token", "verification token for the rpc requests",
            value(args->verify_token)->default_value(string(DEFAULT_VTOKEN)))(
            "t,time-out", "connection time out in seconds",
            value(time_out)->default_value("9"))(
            "k,keep-alive", "keep alive timer in seconds",
            value(keep_alive)->default_value("3"))(
            "help", "print usage");

    opts.add_options("commands")  //
        ("system-info", "show system informations", value(args->system_info))(
            "system-status", "show RAN connection status",
            value(args->system_status))("connection-state",
                                        "show WAN connection status",
                                        value(args->connection_state))(
            "sms-storage-state", "show SMS storage counters",
            value(args->sms_storage_state))(
            "sms-contact-list", "show SMS contacts with last message",
            value(args->sms_contact_list))("sms-content-list",
                                           "show SMSes of a contact with <id>",
                                           value(args->sms_content_list))(
            "n,page", "specify the page number",
            value(args->page)->default_value("1"))(
            "d,detailed", "show information views in deatiled mode",
            value(args->detailed))("send-sms", "sending sms to phone numbers",
                                   value(args->send_sms))(
            "c,content", "used with send sms for sms text",
            value(args->content))(
            "delete-sms", "wihout sms-content-lits delet by contact id",
            value(args->delete_sms))(
            "w,watch", "watching directory for file creation",
            value(args->watch)->implicit_value("."))(
            "m,move-to", "move sended file to this directory",
            value(args->move_to)->implicit_value("."))(
            "r,reprocess", "reprocess files which created arg minutes ago",
            value(reprocess)->default_value("5")
            );

    auto result = std::make_unique<ParseResult>(opts.parse(argc, argv));

    if (result->count("help")) {
        fmt::print("{}", opts.help());
        std::exit(EXIT_SUCCESS);
    }

    trim(log_level);
    trim(args->host);
    trim(args->base_path);
    trim(args->verify_token);
    trim(args->content);

    args->log_level  = spdlog::level::from_str(log_level);
    args->time_out   = seconds{time_out};
    args->keep_alive = seconds{keep_alive};
    args->reprocess  = minutes{reprocess};

    return args;
}

messages::Device DefaultDevice(Args &args) {
    if (args.keep_alive >= args.time_out) {
        fmt::print("keep alive value {} have to be smaller then time out {}",
            args.keep_alive, args.time_out);
        std::exit(EX_USAGE);
    }
    messages::Device http_client{
        args.host, args.port, std::move(args.base_path),
        args.keep_alive, args.time_out, version::v2
    };
    
    http_client.set_default_headers({
        {"Content-Type", "application/json"},
        {"_TclRequestVerificationKey", std::move(args.verify_token)},
        {"_TclRequestVerificationToken", "null"},
        {"Referer", fmt::format("http://{}/index.html", args.host)},
        {"Origin", std::move(args.host)},
    });
    return http_client;
}