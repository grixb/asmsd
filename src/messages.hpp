#ifndef MESSAGES_H
#define MESSAGES_H

#include <cstdint>
#include <ctime>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "nlohmann/json.hpp"

namespace messages {
using std::string;
using std::time_t;
using json = nlohmann::json;

struct SystemInfo {
    using mac_addr_t = std::uint8_t[6];

    static constexpr auto query_str = "GetSystemInfo";

    string     device_name;
    string     hw_version;
    time_t     build_time;
    string     http_api_version;
    string     iccid;
    string     imei;
    int        imeisv;
    string     imsi;
    mac_addr_t mac_address;
};

void from_json(const json& j, SystemInfo& i) {
    j.at("DeviceName").get_to(i.device_name);
    j.at("HwVersion").get_to(i.hw_version);
    j.at("HttpApiVersion").get_to(i.http_api_version);
    j.at("ICCID").get_to(i.iccid);
    j.at("IMEI").get_to(i.imei);

    string tmp{};
    j.at("IMEISV").get_to(tmp);
    i.imeisv = std::stoi(std::move(tmp));

    j.at("IMSI").get_to(i.imsi);

    j.at("BuildTime").get_to(tmp);
    std::tm tm{};
    std::stringstream{std::move(tmp)} >>
        std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    i.build_time = std::mktime(&tm);

    j.at("MacAddress").get_to(tmp);
    for (size_t j{}; j < 6; ++j)
        i.mac_address[j] = std::stoul(tmp.substr(j * 3, 2), nullptr, 16);
}

}  // end namespace messages

namespace fmt {
enum class Presentation { COMPACT, DETAILED };
template <>
struct fmt::formatter<Presentation> {
    using Presentation::COMPACT;
    using Presentation::DETAILED;

    Presentation presentation = COMPACT;

    constexpr auto parse(format_parse_context& ctx)
        -> format_parse_context::iterator {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it == '}') return it;
        if (it != end && *it == 'c') presentation = COMPACT;
        if (it != end && *it++ == 'd') presentation = DETAILED;

        if (it != end && *it != '}') ctx.on_error("invalid format");

        return it;
    }
};

using messages::SystemInfo;

template <>
struct fmt::formatter<SystemInfo> : fmt::formatter<Presentation> {
    auto format(const SystemInfo& info, format_context& ctx) const
        -> format_context::iterator {
        const auto& mac = info.mac_address;

        if (presentation == COMPACT)
            return fmt::format_to(
                ctx.out(),
                "SYS: {} @ {:%Y.%m.%d}, MAC: {:X}:{:X}:{:X}:{:X}:{:X}:{:X}, "
                "API: {}",
                info.hw_version, *std::localtime(&info.build_time),  //
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],      //
                info.http_api_version);

        if (presentation == DETAILED)
            return fmt::format_to(
                ctx.out(),
                "SYSTEM INFO [{}]:\n"
                "\tHW Version:       {}\n"
                "\tMAC Address:      {:X}:{:X}:{:X}:{:X}:{:X}:{:X}\n"
                "\tBuild Time:       {:%Y.%m.%d}\n"
                "\tHTTP API version: {}\n"
                "\tICCID:            {}\n"
                "\tIMEI:             {}\n"
                "\tIMEI sv:          {}\n"
                "\tIMSI:             {}\n",
                info.device_name, info.hw_version,               //
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],  //
                *std::localtime(&info.build_time),               //
                info.http_api_version, info.iccid, info.imei, info.imeisv,
                info.imsi);

        return ctx.out();
    }
};

}  // namespace fmt

namespace messages {
using std::string_view;
using namespace std::string_view_literals;

static constexpr auto UNKNOWN = "UNKNOWN"sv;

struct SystemStatus {
    static constexpr auto query_str = "GetSystemStatus";

    static constexpr auto NO_SERVICE_SV = "NO Service"sv;
    static constexpr auto GPRS_SV       = "GPRS [2G]"sv;
    static constexpr auto EDGE_SV       = "EDGE [2G]"sv;
    static constexpr auto HSPA_SV       = "HSPA [3G]"sv;
    static constexpr auto HSUPA_SV      = "HSUPA [3G]"sv;
    static constexpr auto UMTS_SV       = "UMTS [3G]"sv;
    static constexpr auto HSPAP_SV      = "HSPA+ [3G+]"sv;
    static constexpr auto DCHSPAP_SV    = "DC HSPA+ [3G+]"sv;
    static constexpr auto LTE_SV        = "LTE [4G]"sv;
    static constexpr auto LTEP_SV       = "LTE+ [4G+]"sv;

    enum NetworkType : std::uint8_t {
        NO_SERVICE,
        GPRS,
        EDGE,  // 2G
        HSPA,
        HSUPA,
        UMTS,  // 3G
        HSPA_PLUS,
        DC_HSPA_PLUS,  // 3G+
        LTE,           // 4G
        LTE_PLUS,      // 4G+
    };

    static const string_view& as_strv(NetworkType nett) {
        switch (nett) {
            case NO_SERVICE:
                return NO_SERVICE_SV;
            case GPRS:
                return GPRS_SV;
            case EDGE:
                return EDGE_SV;
            case HSPA:
                return HSPA_SV;
            case HSUPA:
                return HSUPA_SV;
            case UMTS:
                return UMTS_SV;
            case HSPA_PLUS:
                return HSPAP_SV;
            case DC_HSPA_PLUS:
                return DCHSPAP_SV;
            case LTE:
                return LTE_SV;
            case LTE_PLUS:
                return LTEP_SV;
            default:
                return UNKNOWN;
        }
    }

    static constexpr auto DISCONNECTED_SV  = "disconnected"sv;
    static constexpr auto CONNECTING_SV    = "connecting..."sv;
    static constexpr auto CONNECTED_SV     = "connected"sv;
    static constexpr auto DISCONNECTING_SV = "disconnecting..."sv;

    enum ConnectionStatus : std::uint8_t {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

    static const string_view& as_strv(ConnectionStatus cs) {
        switch (cs) {
            case DISCONNECTED:
                return DISCONNECTED_SV;
            case CONNECTING:
                return CONNECTED_SV;
            case CONNECTED:
                return CONNECTED_SV;
            case DISCONNECTING:
                return DISCONNECTING_SV;
            default:
                return UNKNOWN;
        }
    }

    static constexpr auto DISABLED_SV = "disabled"sv;
    static constexpr auto FULL_SV     = "full"sv;
    static constexpr auto NORMAL_SV   = "normal"sv;
    static constexpr auto NEW_SV      = "new"sv;

    enum SmsState : std::uint8_t { DISABLED, FULL, NORMAL, NEW };

    static const string_view& as_strv(SmsState smss) {
        switch (smss) {
            case DISABLED:
                return DISABLED_SV;
            case FULL:
                return FULL_SV;
            case NORMAL:
                return NORMAL_SV;
            case NEW:
                return NEW_SV;
            default:
                return UNKNOWN;
        }
    }

    string           network_name;
    int              signal_strength;
    int              conprof_error;
    int              clear_code;
    int              m_pdp_reject_count;
    NetworkType      network_type;
    ConnectionStatus connection_status;
    SmsState         sms_state;
    bool             roaming;
    bool             domestic_roaming;

    inline const string_view& network_type_sv() const {
        return as_strv(network_type);
    }
    inline const string_view& connection_status_sv() const {
        return as_strv(connection_status);
    }
    inline const string_view& sms_state_sv() const {
        return as_strv(sms_state);
    }
};

void from_json(const json& j, SystemStatus& ss) {
    j.at("NetworkType").get_to(ss.network_type);
    j.at("NetworkName").get_to(ss.network_name);
    int tmp;
    j.at("Roaming").get_to(tmp);
    ss.roaming = tmp > 0;
    j.at("Domestic_Roaming").get_to(tmp);
    ss.domestic_roaming = tmp > 0;
    j.at("SignalStrength").get_to(ss.signal_strength);
    j.at("ConnectionStatus").get_to(ss.connection_status);
    j.at("Conprofileerror").get_to(ss.conprof_error);
    j.at("ClearCode").get_to(ss.clear_code);
    j.at("mPdpRejectCount").get_to(ss.m_pdp_reject_count);
    j.at("SmsState").get_to(ss.sms_state);
}
}  // namespace messages

namespace fmt {
using messages::SystemStatus;
using namespace std::string_view_literals;
constexpr auto ENABLED  = "ENABLED"sv;
constexpr auto DISABLED = "DISABLED"sv;

template <>
struct fmt::formatter<SystemStatus> : fmt::formatter<Presentation> {
    auto format(const SystemStatus& status, format_context& ctx) const
        -> format_context::iterator {
        if (presentation == COMPACT)
            return fmt::format_to(
                ctx.out(),
                "NET: {} @ {} is {} with signal strength {}, SMS: {}",
                status.network_type_sv(), status.network_name,
                status.connection_status_sv(), status.signal_strength,
                status.sms_state_sv());

        if (presentation == DETAILED)
            return fmt::format_to(ctx.out(),
                                  "SYSTEM STATUS [{}]:\n"
                                  "\tNetwork Name:     {}\n"
                                  "\tNetwork Type:     {}\n"
                                  "\tSignal Strength:  {}\n"
                                  "\tSMS State:        {}\n"
                                  "\tRoaming:          {}\n"
                                  "\tConnection Error: {}\n"
                                  "\tClear Code:       {}\n",
                                  status.connection_status_sv(),
                                  status.network_name, status.network_type_sv(),
                                  status.signal_strength, status.sms_state_sv(),
                                  status.roaming ? ENABLED : DISABLED,
                                  status.conprof_error, status.clear_code);

        return ctx.out();
    }
};
}  // namespace fmt

namespace messages {
struct SmsStorageState {
    static constexpr auto query_str = "GetSMSStorageState";

    int unread_report;
    int left_count;
    int max_count;
    int use_count;
    int unread_count;
};

void from_json(const json& j, SmsStorageState& ss) {
    j.at("UnreadReport").get_to(ss.unread_report);
    j.at("LeftCount").get_to(ss.left_count);
    j.at("MaxCount").get_to(ss.max_count);
    j.at("TUseCount").get_to(ss.use_count);
    j.at("UnreadSMSCount").get_to(ss.unread_count);
}
}  // namespace messages

namespace fmt {
using messages::SmsStorageState;
template <>
struct fmt::formatter<SmsStorageState> : fmt::formatter<Presentation> {
    auto format(const SmsStorageState& smss, format_context& ctx) const
        -> format_context::iterator {
        if (presentation == COMPACT)
            return fmt::format_to(
                ctx.out(),
                "SMS Storage counters: Use: {}, unread/left/max: {}/{}/{}",
                smss.use_count, smss.unread_count, smss.left_count,
                smss.max_count);
        return ctx.out();
    }
};
}  // namespace fmt

namespace messages {
using std::vector;
struct Page {
    using named_parameter = std::map<std::string, json>;

    int page;

    Page(int p) : page{p} {}
    Page() = default;

    operator named_parameter() const { return {{"Page", page}}; }
};
void to_json(json& j, const Page& p) { j = json{{"Page", p.page}}; }
void from_json(const json& j, Page& p) { j.at("Page").get_to(p.page); }

template <typename CRTP>
struct Pagination : Page {
    using crtp_t = CRTP;
    int total_pages;
};

template <typename CRTP>
void from_json(const json& j, Pagination<CRTP>& p) {
    j.get_to(static_cast<Page&>(*p));
    j.at("TotalPageCount").get_to(p.total_pages);
    j.get_to(static_cast<CRTP&>(*p));
}

struct SmsContact {
    static constexpr auto READ_SV   = "read"sv;
    static constexpr auto UNREAD_SV = "unread"sv;
    static constexpr auto SENT_SV   = "sent"sv;
    static constexpr auto FAILED_SV = "failed"sv;
    static constexpr auto REPORT_SV = "report"sv;
    static constexpr auto FLASH_SV  = "flash"sv;
    static constexpr auto DRAFT_SV  = "draft"sv;

    enum SmsType : std::uint8_t {
        READ,
        UNREAD,
        SENT,
        FAILED,
        REPORT,
        FLASH,
        DRAFT
    };

    static const string_view& as_strv(SmsType smst) {
        switch (smst) {
            case READ:
                return READ_SV;
            case UNREAD:
                return UNREAD_SV;
            case SENT:
                return SENT_SV;
            case FAILED:
                return FAILED_SV;
            case REPORT:
                return REPORT_SV;
            case FLASH:
                return FLASH_SV;
            case DRAFT:
                return DRAFT_SV;
            default:
                return UNKNOWN;
        }
    }

    int            contact_id;
    vector<string> phone_numbers;
    int            sms_id;
    SmsType        sms_type;
    bool           sms_report;
    bool           report_status;
    int            report_id;
    string         sms_content;
    time_t         sms_time;
    time_t         report_time;
    int            time_zone;
    int            unread_count;
    int            sms_count;

    inline const string_view& sms_type_sv() const { return as_strv(sms_type); }
};

void from_json(const json& j, SmsContact& smsc) {
    j.at("ContactId").get_to(smsc.contact_id);
    j.at("PhoneNumber").get_to(smsc.phone_numbers);
    j.at("SMSId").get_to(smsc.sms_id);
    j.at("SMSType").get_to(smsc.sms_type);

    int report;
    j.at("ReportStatus").get_to(report);
    smsc.report_status = report > 0;
    j.at("sms_report").get_to(report);
    smsc.sms_report = report > 0;
    j.at("report_id").get_to(smsc.report_id);
    j.at("SMSContent").get_to(smsc.sms_content);

    string tmp{};
    j.at("SMSTime").get_to(tmp);
    std::tm tm{};
    std::stringstream{std::move(tmp)} >>
        std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    smsc.sms_time = std::mktime(&tm);

    j.at("report_time").get_to(tmp);
    std::stringstream{std::move(tmp)} >>
        std::get_time(&tm, "%Y-%b-%d %H:%M:%S");
    smsc.report_time = std::mktime(&tm);

    j.at("SMSTimezone").get_to(smsc.time_zone);
    j.at("UnreadCount").get_to(smsc.unread_count);
    j.at("TSMSCount").get_to(smsc.sms_count);
}

struct SmsContactList : Pagination<SmsContactList> {
    static constexpr auto query_str = "GetSMSContactList";
    vector<SmsContact>    contacts;
};

void from_json(const json& j, SmsContactList& smsc) {
    j.at("SMSContactList").get_to(smsc.contacts);
}
}  // namespace messages

namespace fmt {
using messages::SmsContact;
using messages::SmsContactList;

template <>
struct fmt::formatter<SmsContact> : fmt::formatter<Presentation> {
    auto format(const SmsContact& cntc, format_context& ctx) const
        -> format_context::iterator {
        const auto empty = cntc.phone_numbers.empty();

        if (presentation == COMPACT)
            return fmt::format_to(
                ctx.out(), "{} [{}] {} @ {:%Y.%m.%d. %H:%M}\n\t{}\n",
                empty ? "<invalid numb>" : cntc.phone_numbers[0],
                cntc.sms_count, cntc.sms_type_sv(),
                *std::localtime(&cntc.sms_time), cntc.sms_content);

        return ctx.out();
    }
};

template <>
struct fmt::formatter<SmsContactList> : fmt::formatter<Presentation> {
    auto format(const SmsContactList& clst, format_context& ctx) const
        -> format_context::iterator {
        if (presentation == COMPACT) {
            auto out = fmt::format_to(ctx.out(), "SMS CONTACT LIST [{}/{}]:\n",
                                      clst.page + 1, clst.total_pages);

            for (const SmsContact& ctct : clst.contacts) {
                out = fmt::format_to(out, " {}\n", ctct);
            }

            return out;
        }

        return ctx.out();
    }
};
}  // namespace fmt

#endif
