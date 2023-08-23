#include "args.hpp"

#include "cxxopts.hpp"
#include "spdlog/spdlog.h"
#include "fmt/core.h"
#include "string_trim.hpp"

namespace {
using namespace std::string_view_literals;
constexpr auto DEFAULT_VTOKEN =
    "KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf"sv;
}  // namespace

namespace aux {
struct Args::_Repr {
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

Args::Args()  = default;
Args::~Args() = default;

void Args::parse(int argc, const char* argv[]) {
    using cxxopts::Options;
    using cxxopts::ParseResult;
    using cxxopts::value;

    if (!_repr) _repr = std::make_unique<_Repr>();

    auto options_allocated =
        std::make_unique<Options>("asmsd", "ASMSD - Alcatel SMS Daemon");
    auto& opts = *options_allocated;

    string log_level{};
    long   time_out{};
    long   keep_alive{};
    long   reprocess{};

    opts.set_width(90).add_options()  //
        ("l,log-level", "log level", value(log_level)->default_value("info"))(
            "h,host", "hostname or ip address of the device",
            value(_repr->host)->default_value("192.168.1.1"))(
            "p,port", "target port number of the http connection",
            value(_repr->port)->default_value("80"))(
            "b,base-path", "base path of the json rpc endpoint",
            value(_repr->base_path)->default_value("/jrd/webapi"))(
            "v,verify-token", "verification token for the rpc requests",
            value(_repr->verify_token)->default_value(string(DEFAULT_VTOKEN)))(
            "t,time-out", "connection time out in seconds",
            value(time_out)->default_value("9"))(
            "k,keep-alive", "keep alive timer in seconds",
            value(keep_alive)->default_value("3"))("help", "print usage");

    opts.add_options("commands")  //
        ("system-info", "show system informations", value(_repr->system_info))(
            "system-status", "show RAN connection status",
            value(_repr->system_status))("connection-state",
                                         "show WAN connection status",
                                         value(_repr->connection_state))(
            "sms-storage-state", "show SMS storage counters",
            value(_repr->sms_storage_state))(
            "sms-contact-list", "show SMS contacts with last message",
            value(_repr->sms_contact_list))("sms-content-list",
                                            "show SMSes of a contact with <id>",
                                            value(_repr->sms_content_list))(
            "n,page", "specify the page number",
            value(_repr->page)->default_value("1"))(
            "d,detailed", "show information views in deatiled mode",
            value(_repr->detailed))("send-sms", "sending sms to phone numbers",
                                    value(_repr->send_sms))(
            "c,content", "used with send sms for sms text",
            value(_repr->content))(
            "delete-sms", "wihout sms-content-lits delet by contact id",
            value(_repr->delete_sms))("w,watch",
                                      "watching directory for file creation",
                                      value(_repr->watch)->implicit_value("."))(
            "m,move-to", "move sended file to this directory",
            value(_repr->move_to)->implicit_value("."))(
            "r,reprocess", "reprocess files which created arg minutes ago",
            value(reprocess)->default_value("5"));

    auto result = std::make_unique<ParseResult>(opts.parse(argc, argv));

    if (result->count("help")) {
        fmt::print("{}", opts.help());
        std::exit(EXIT_SUCCESS);
    }

    trim(log_level);
    trim(_repr->host);
    trim(_repr->base_path);
    trim(_repr->verify_token);
    trim(_repr->content);

    _repr->log_level  = spdlog::level::from_str(log_level);
    _repr->time_out   = seconds{time_out};
    _repr->keep_alive = seconds{keep_alive};
    _repr->reprocess  = minutes{reprocess};
}

Args::vector<Args::string>&& Args::send_sms() && noexcept {
    return std::move(_repr->send_sms);
}

Args::string&& Args::content() && noexcept {
    return std::move(_repr->content);
}

Args::string&& Args::base_path() && noexcept {
    return std::move(_repr->base_path);
}

Args::string&& Args::verify_token() && noexcept {
    return std::move(_repr->verify_token);
}

Args::string&& Args::host() && noexcept { return std::move(_repr->host); }

const Args::level_enum& Args::log_level() const& noexcept {
    return _repr->log_level;
}

const Args::string& Args::host() const& noexcept { return _repr->host; }
const int&          Args::port() const& noexcept { return _repr->port; }
const Args::string& Args::base_path() const& noexcept {
    return _repr->base_path;
}
const Args::string& Args::verify_token() const& noexcept {
    return _repr->verify_token;
}
const bool& Args::detailed() const& noexcept { return _repr->detailed; }
const bool& Args::system_info() const& noexcept { return _repr->system_info; }
const bool& Args::system_status() const& noexcept {
    return _repr->system_status;
}
const bool& Args::connection_state() const& noexcept {
    return _repr->connection_state;
}
const bool& Args::sms_storage_state() const& noexcept {
    return _repr->sms_storage_state;
}
const bool& Args::sms_contact_list() const& noexcept {
    return _repr->sms_contact_list;
}
const int& Args::sms_content_list() const& noexcept {
    return _repr->sms_content_list;
}
const size_t& Args::page() const& noexcept { return _repr->page; }
const Args::vector<Args::string>& Args::send_sms() const& noexcept {
    return _repr->send_sms;
}
const Args::string& Args::content() const& noexcept { return _repr->content; }
const int&      Args::delete_sms() const& noexcept { return _repr->delete_sms; }
const fs::path& Args::watch() const& noexcept { return _repr->watch; }
const fs::path& Args::move_to() const& noexcept { return _repr->move_to; }
const Args::seconds& Args::time_out() const& noexcept {
    return _repr->time_out;
}
const Args::seconds& Args::keep_alive() const& noexcept {
    return _repr->keep_alive;
}
const Args::minutes& Args::reprocess() const& noexcept {
    return _repr->reprocess;
}
}  // namespace aux