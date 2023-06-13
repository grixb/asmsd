#include <memory>
#include <exception>
#include <string>
#include <string_view>
#include <cstdlib>
#include <sysexits.h>
#include <map>

#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include "cxxopts.hpp"
#include "jsonrpccxx/client.hpp"
#include "nlohmann/json.hpp"

#include "httpclientconnector.hpp"
#include "string_trim.hpp"

using std::string;
using namespace std::string_view_literals;
using cxxopts::ParseResult;
using jsonrpccxx::version;
using json = nlohmann::json;
using JsonObject = std::map<string, json>;

constexpr auto DEFAULT_VTOKEN =
"KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf"sv;

struct Args {
    spdlog::level::level_enum log_level;
    string                    host;
    int                       port;
    string                    base_path;
    string                    verify_token;
};
std::unique_ptr<Args> parse(int argc, const char* argv[]);

int main(int argc, const char* argv[]) {
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
        spdlog::info("log levelt seted to: {}",
            spdlog::level::to_string_view(spdlog::get_level()));
    }

    try {
        spdlog::info("connecting to host: {}, with: {}",
            args->host, args->port);

        staff::HttpClientConnector http_client{
            args->host,
            args->port,
            std::move(args->base_path)
        };
        http_client.set_default_headers({
            {"Content-Type", "application/json"},
            {"_TclRequestVerificationKey", std::move(args->verify_token)},
            {"_TclRequestVerificationToken", "null"},
            {"Referer", fmt::format("http://{}/index.html", args->host)},
            {"Origin", std::move(args->host)},
        });

        jsonrpccxx::JsonRpcClient client(http_client, version::v2);
        const auto ret = client.CallMethod<JsonObject>("1", "GetSystemInfo");

        fmt::print("RESPONSE:\n");
        for (const auto& [k, v] : ret)
            fmt::print("\t{}: \t{}\n", k, v);


    } catch (const std::exception &e) {
        spdlog::critical("json rpc request error: {}", e.what());
        std::exit(EX_SOFTWARE);
    }

    std::exit(EXIT_SUCCESS);
}

std::unique_ptr<Args> parse(int argc, const char* argv[]) {
    using cxxopts::Options;
    using cxxopts::value;

    auto options_allocated = 
        std::make_unique<Options>("asmsd", " - alcatel sms daemon");
    auto& opts = *options_allocated;

    auto args = std::make_unique<Args>();
    string log_level{};
    opts.add_options()
    ("l,log-level", "log level", 
        value(log_level)->default_value("info"))
    ("h,host", "hostname or ip address of the device", 
        value(args->host)->default_value("localhost"))
    ("p,port", "target port number of the http connection", 
        value(args->port)->default_value("80"))
    ("b,base-path", "base path of the json rpc endpoint",
        value(args->base_path)->default_value("/jrd/webapi"))
    ("v,verify-token", "verification token for the rpc requests",
        value(args->verify_token)->default_value(string(DEFAULT_VTOKEN)))
    ("help", "print usage");

    auto result = 
        std::make_unique<ParseResult>(opts.parse(argc, argv));

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