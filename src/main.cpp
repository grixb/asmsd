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

#include "cxxopts.hpp"
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

using std::string;
using std::vector;
using namespace std::string_view_literals;
using namespace std::chrono_literals;
using cxxopts::ParseResult;
using jsonrpccxx::version;
using inotify::BuildNotifier;

namespace fs = std::filesystem;

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
};
std::unique_ptr<Args> parse(int argc, const char *argv[]);

staff::HttpClientConnector DefaultClient(Args &args);
static constexpr auto      INVALID_MSG =
    R"(invalid error response: "code" (negative number) and "message" (string) are required)";

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
        spdlog::debug("try connect to host: {}, with: {}", args->host,
                      args->port);

        auto                      http_client = DefaultClient(*args);
        jsonrpccxx::JsonRpcClient client(http_client, version::v2);

        bool specified{};

        if (args->system_info) {
            const auto sys_inf = std::make_unique<SystemInfo>(
                client.CallMethod<json>("1", SystemInfo::query_str));
            if (args->detailed)
                fmt::print("{:d}\n", *sys_inf);
            else
                fmt::print("{}\n", *sys_inf);
            specified = true;
        }

        if (args->system_status) {
            const auto sys_status = std::make_unique<SystemStatus>(
                client.CallMethod<json>("2", SystemStatus::query_str));
            if (args->detailed)
                fmt::print("{:d}\n", *sys_status);
            else
                fmt::print("{}\n", *sys_status);
            specified = true;
        }

        if (args->connection_state) {
            const auto con_state = std::make_unique<ConnectionState>(
                client.CallMethod<json>("3", ConnectionState::query_str));
            if (args->detailed)
                fmt::print("{:d}\n", *con_state);
            else
                fmt::print("{}\n", *con_state);
            specified = true;
        }

        if (args->sms_storage_state) {
            const auto sms_state = client.CallMethod<SmsStorageState>(
                "4", SmsStorageState::query_str);
            fmt::print("{}\n", sms_state);
            specified = true;
        }

        if (args->sms_contact_list) {
            const auto sms_clist = client.CallMethodNamed<SmsContactList>(
                "5", SmsContactList::query_str,
                Page(args->page > 0 ? args->page - 1 : 0));
            fmt::print("{}\n", sms_clist);
            specified = true;
        }

        if (args->sms_content_list > 0) {
            if (args->delete_sms > 0) try {
                    client.CallMethodNamed<json>(
                        "10", DeleteSms::query_str,
                        DeleteSms(args->sms_content_list, args->delete_sms));
                } catch (const jsonrpccxx::JsonRpcException &je) {
                    if (je.Code() != jsonrpccxx::internal_error &&
                        je.Message() != INVALID_MSG)
                        throw je;
                }
            else {
                const auto sms_clist = client.CallMethodNamed<SmsContentList>(
                    "6", SmsContentList::query_str,
                    GetSmsContentList(args->sms_content_list,
                                      args->page > 0 ? args->page - 1 : 0));
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

            client.CallMethodNamed<json>(
                "7", SendSms::query_str,
                SendSms(std::move(args->send_sms), std::move(args->content)));

            auto res =
                client.CallMethod<SendSmsResult>("8", SendSmsResult::query_str);

            if (res.send_status == SendSmsResult::SENDING) {
                while (res.send_status != SendSmsResult::SENDING) {
                    spdlog::debug("... still seinding, waiting 1 sec ...");
                    std::this_thread::sleep_for(1s);
                    res = client.CallMethod<SendSmsResult>(
                        "9", SendSmsResult::query_str);
                }
            }

            spdlog::debug("SMS send status: {}", res.send_status_sv());

            specified = true;
        }

        if (args->delete_sms > 0 && args->sms_content_list <= 0) {
            try {
                client.CallMethodNamed<json>("10", DeleteSms::query_str,
                                             DeleteSms(args->delete_sms));
            } catch (const jsonrpccxx::JsonRpcException &je) {
                if (je.Code() != jsonrpccxx::internal_error &&
                    je.Message() != INVALID_MSG)
                    throw je;
            }
            specified = true;
        }

        if (!args->watch.empty()) {
            const auto watch_string = args->watch.string();
            if(!fs::is_directory(args->watch)) {
                spdlog::error("watching path: {} is not a directory", watch_string);
                std::exit(EX_USAGE);
            }

            spdlog::debug("start watching directory for file creation: {}", watch_string);
            auto notifier = BuildNotifier()
                .watchPathRecursively(args->watch)
                .onEvents(
                    {inotify::Event::create}, 
                    [&client, &mv_to = args->move_to](inotify::Notification notif){
                    const auto path_string = notif.path.string();
                    spdlog::debug("new file created at {}", path_string);

                    std::ifstream in_file{notif.path};
                    if (!in_file) {
                        spdlog::error("unable to open file: {}", path_string);
                        return;
                    }
                    std::stringstream buffer{};
                    buffer << in_file.rdbuf();
                    auto phone_number = notif.path.stem().string();

                    spdlog::debug("sending SMS to: {}", phone_number);
                    client.CallMethodNamed<json>(
                        "11", SendSms::query_str,
                        SendSms({std::move(phone_number)}, buffer.str()));

                    auto res =
                        client.CallMethod<SendSmsResult>("12", SendSmsResult::query_str);

                    if (res.send_status == SendSmsResult::SENDING) {
                        while (res.send_status != SendSmsResult::SENDING) {
                            spdlog::debug("... still seinding, waiting 1 sec ...");
                            std::this_thread::sleep_for(1s);
                            res = client.CallMethod<SendSmsResult>(
                                "13", SendSmsResult::query_str);
                        }
                    }

                    if (!mv_to.empty()) {
                        spdlog::debug("moving file {} to {}", 
                            notif.path.filename().string(),
                            mv_to.string());
                        fs::rename(notif.path, mv_to/notif.path.filename());
                    }

                });
            
            spdlog::debug("registring stop signal: press ^C to stop watching...");
            SignalScope<SIGINT> sigint([&notifier](int sig){
                spdlog::debug("got signal {}, stopping watch...", sig);
                notifier.stop();
            });
            spdlog::debug("start running notifier thread...");
            std::thread thread([&](){ notifier.run(); });

            thread.join();

            specified = true;
        }

        if (!specified)
            fmt::print(stderr,
                       "no command specified, see usage: asmsd --help\n");

    } catch (const std::exception &e) {
        spdlog::critical("json rpc request error: {}", e.what());
        std::exit(EX_SOFTWARE);
    }

    std::exit(EXIT_SUCCESS);
}

std::unique_ptr<Args> parse(int argc, const char *argv[]) {
    using cxxopts::Options;
    using cxxopts::value;

    auto options_allocated =
        std::make_unique<Options>("asmsd", "ASMSD - Alcatel SMS Daemon");
    auto &opts = *options_allocated;

    auto   args = std::make_unique<Args>();
    string log_level{};
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
            value(args->move_to)->implicit_value("."));

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

    args->log_level = spdlog::level::from_str(log_level);

    return args;
}

staff::HttpClientConnector DefaultClient(Args &args) {
    staff::HttpClientConnector http_client{args.host, args.port,
                                           std::move(args.base_path)};
    http_client.set_default_headers({
        {"Content-Type", "application/json"},
        {"_TclRequestVerificationKey", std::move(args.verify_token)},
        {"_TclRequestVerificationToken", "null"},
        {"Referer", fmt::format("http://{}/index.html", args.host)},
        {"Origin", std::move(args.host)},
    });
    return http_client;
}