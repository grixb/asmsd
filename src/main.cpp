#include <sysexits.h>

#include <cstdlib>
#include <exception>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "cxxopts.hpp"
#include "fmt/core.h"
#include "httpclientconnector.hpp"
#include "jsonrpccxx/client.hpp"
#include "messages.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "string_trim.hpp"

using std::string;
using namespace std::string_view_literals;
using cxxopts::ParseResult;
using jsonrpccxx::version;
using json       = nlohmann::json;
using JsonObject = std::map<string, json>;

using namespace messages;

constexpr auto DEFAULT_VTOKEN =
    "KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf"sv;

struct Args {
    spdlog::level::level_enum log_level;
    string                    host;
    int                       port;
    string                    base_path;
    string                    verify_token;
    bool                      detailed;
    bool                      system_info;
    bool                      system_status;
    bool                      sms_storage_state;
    int                       sms_contact_list;
};
std::unique_ptr<Args> parse(int argc, const char *argv[]);

staff::HttpClientConnector DefaultClient(Args &args);

int main(int argc, const char *argv[]) {
    spdlog::info("asmsd - Alcatel (dongle) SMS Daemon");
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

    if (spdlog::get_level() != args->log_level) {
        spdlog::set_level(args->log_level);
        spdlog::info("log levelt set to: {}",
                     spdlog::level::to_string_view(spdlog::get_level()));
    }

    try {
        spdlog::info("try connect to host: {}, with: {}", args->host,
                     args->port);

        auto                      http_client = DefaultClient(*args);
        jsonrpccxx::JsonRpcClient client(http_client, version::v2);

        bool specified{};

        if (args->system_info) {
            const auto sys_inf =
                client.CallMethod<SystemInfo>("1", SystemInfo::query_str);
            if (args->detailed)
                fmt::print("{:d}\n", sys_inf);
            else
                fmt::print("{}\n", sys_inf);
            specified = true;
        }

        if (args->system_status) {
            const auto sys_status =
                client.CallMethod<SystemStatus>("2", SystemStatus::query_str);
            if (args->detailed)
                fmt::print("{:d}\n", sys_status);
            else
                fmt::print("{}\n", sys_status);
            specified = true;
        }

        if (args->sms_storage_state) {
            const auto sms_state = client.CallMethod<SmsStorageState>(
                "3", SmsStorageState::query_str);
            fmt::print("{}\n", sms_state);
            specified = true;
        }

        if (args->sms_contact_list > 0) {
            const auto sms_clist = client.CallMethodNamed<SmsContactList>(
                "4", SmsContactList::query_str,
                Page(args->sms_contact_list - 1));
            fmt::print("{}\n", sms_clist);
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
            value(args->host)->default_value("localhost"))(
            "p,port", "target port number of the http connection",
            value(args->port)->default_value("80"))(
            "b,base-path", "base path of the json rpc endpoint",
            value(args->base_path)->default_value("/jrd/webapi"))(
            "v,verify-token", "verification token for the rpc requests",
            value(args->verify_token)->default_value(string(DEFAULT_VTOKEN)))(
            "help", "print usage");

    opts.add_options("commands")  //
        ("system-info", "show system informations", value(args->system_info))(
            "system-status", "show network connection status",
            value(args->system_status))("sms-storage-state",
                                        "show SMS storage counters",
                                        value(args->sms_storage_state))(
            "sms-contact-list", "show SMS contacts with last message",
            value(args->sms_contact_list)->implicit_value("1"))(
            "detailed", "show information views in deatiled mode",
            value(args->detailed));

    auto result = std::make_unique<ParseResult>(opts.parse(argc, argv));

    if (result->count("help")) {
        fmt::print("{}", opts.help());
        std::exit(EXIT_SUCCESS);
    }

    trim(log_level);
    trim(args->host);
    trim(args->base_path);
    trim(args->verify_token);

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