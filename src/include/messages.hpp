#ifndef MESSAGES_H
#define MESSAGES_H

#include <chrono>
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
using namespace std::string_view_literals;

using std::string;
using std::string_view;
using std::time_t;
using std::chrono::seconds;
using std::vector;

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

    inline const std::tm& build_time_ltm() const {
        return *std::localtime(&build_time);
    }

    SystemInfo(const json&& j);
};

void from_json(const json& j, SystemInfo& i);

static constexpr auto UNKNOWN = "UNKNOWN"sv;

struct SystemStatus {
    static constexpr auto query_str = "GetSystemStatus";

    static constexpr auto NO_SERVICE_SV    = "NO Service"sv;
    static constexpr auto GPRS_SV          = "GPRS [2G]"sv;
    static constexpr auto EDGE_SV          = "EDGE [2G]"sv;
    static constexpr auto HSPA_SV          = "HSPA [3G]"sv;
    static constexpr auto HSUPA_SV         = "HSUPA [3G]"sv;
    static constexpr auto UMTS_SV          = "UMTS [3G]"sv;
    static constexpr auto HSPAP_SV         = "HSPA+ [3G+]"sv;
    static constexpr auto DCHSPAP_SV       = "DC HSPA+ [3G+]"sv;
    static constexpr auto LTE_SV           = "LTE [4G]"sv;
    static constexpr auto LTEP_SV          = "LTE+ [4G+]"sv;
    static constexpr auto DISCONNECTED_SV  = "disconnected"sv;
    static constexpr auto CONNECTING_SV    = "connecting..."sv;
    static constexpr auto CONNECTED_SV     = "connected"sv;
    static constexpr auto DISCONNECTING_SV = "disconnecting..."sv;
    static constexpr auto DISABLED_SV      = "disabled"sv;
    static constexpr auto FULL_SV          = "full"sv;
    static constexpr auto NORMAL_SV        = "normal"sv;
    static constexpr auto NEW_SV           = "new"sv;

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

    enum ConnectionStatus : std::uint8_t {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

    enum SmsState : std::uint8_t { DISABLED, FULL, NORMAL, NEW };

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

    static const string_view& as_strv(ConnectionStatus cs) noexcept;
    static const string_view& as_strv(SmsState smss) noexcept;
    static const string_view& as_strv(NetworkType nett) noexcept;

    inline const string_view& network_type_sv() const noexcept {
        return as_strv(network_type);
    }
    inline const string_view& connection_status_sv() const noexcept {
        return as_strv(connection_status);
    }
    inline const string_view& sms_state_sv() const noexcept {
        return as_strv(sms_state);
    }

    SystemStatus(const json&& j);
};

void from_json(const json& j, SystemStatus& ss);

struct SmsStorageState {
    static constexpr auto query_str = "GetSMSStorageState";

    int unread_report;
    int left_count;
    int max_count;
    int use_count;
    int unread_count;
};

void from_json(const json& j, SmsStorageState& ss);

using ConnectionStatus = SystemStatus::ConnectionStatus;

struct ConnectionState {
    static constexpr auto query_str = "GetConnectionState";

    ConnectionStatus connection_status;
    int              conprof_error;
    int              clear_code;
    int              m_pdp_reject_count;
    string           ipv4_address;
    string           ipv6_address;
    size_t           dl_speed;
    size_t           ul_speed;
    size_t           dl_rate;
    size_t           ul_rate;
    size_t           dl_bytes;
    size_t           ul_bytes;
    seconds          connection_time;

    inline const string_view& connection_status_sv() const noexcept {
        return SystemStatus::as_strv(connection_status);
    }

    ConnectionState(const json&& j);
};

void from_json(const json& j, ConnectionState& cs);

struct Page {
    using named_parameter = std::map<std::string, json>;

    size_t page;

    Page(size_t p) : page{p} {}
    Page() = default;

    operator named_parameter() const { return {{"Page", page}}; }
};

void from_json(const json& j, Page& p);

template <typename CRTP>
struct Pagination : Page {
    using crtp_t = CRTP;
    int total_pages;
};

template <typename CRTP>
void from_json(const json& j, Pagination<CRTP>& p) {
    j.get_to(static_cast<Page&>(p));
    j.at("TotalPageCount").get_to(p.total_pages);
}

struct GetSmsContentList : Page {
    int contact_id;
    GetSmsContentList(int id, size_t p) : Page{p}, contact_id{id} {}
    operator named_parameter() const {
        return {{"Page", page}, {"ContactId", contact_id}};
    }
};

struct SmsContent {
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

    int     sms_id;
    SmsType sms_type;
    bool    sms_report;
    bool    report_status;
    int     report_id;
    string  sms_content;
    time_t  sms_time;
    time_t  report_time;
    int     time_zone;

    static const string_view& as_strv(SmsType smst) noexcept;

    inline const string_view& sms_type_sv() const noexcept {
        return as_strv(sms_type);
    }
    inline const std::tm& sms_time_ltm() const noexcept {
        return *std::localtime(&sms_time);
    }
    inline const std::tm& report_time_ltm() const noexcept {
        return *std::localtime(&report_time);
    }
};

void from_json(const json& j, SmsContent& smsc);

struct SmsContentList : Pagination<SmsContentList> {
    static constexpr auto query_str = "GetSMSContentList";

    vector<SmsContent> contents;
    int                contact_id;
    vector<string>     phone_numbers;
};

void from_json(const json& j, SmsContentList& smsc);

struct SmsContact : SmsContent {
    int            contact_id;
    vector<string> phone_numbers;
    size_t         unread_count;
    int            sms_count;
};

void from_json(const json& j, SmsContact& smsc);

struct SmsContactList : Pagination<SmsContactList> {
    static constexpr auto query_str = "GetSMSContactList";

    vector<SmsContact> contacts;
};

void from_json(const json& j, SmsContactList& smsc);

struct SendSms {
    using named_parameter = std::map<std::string, json>;
    using clock           = std::chrono::system_clock;

    static constexpr auto query_str = "SendSMS";

    int            sms_id;
    string         sms_content;
    vector<string> phone_numbers;
    time_t         sms_time;

    SendSms(vector<string> nums, string content)
        : sms_id{-1},
          sms_content{std::move(content)},
          phone_numbers{std::move(nums)} {
        sms_time = clock::to_time_t(clock::now());
    }

    operator named_parameter() const {
        return {{"SMSId", sms_id},
                {"SMSContent", sms_content},
                {"PhoneNumber", phone_numbers},
                {"SMSTime", fmt::format("{:%Y-%m-%d %H:%M:%S}",
                                        *std::localtime(&sms_time))}};
    }
};

struct SendSmsResult {
    static constexpr auto query_str = "GetSendSMSResult";

    static constexpr auto NONE_SV         = "none"sv;
    static constexpr auto SENDING_SV      = "sending..."sv;
    static constexpr auto SUCCESS_SV      = "success"sv;
    static constexpr auto FAIL_SENDING_SV = "retring..."sv;
    static constexpr auto FULL_SV         = "memory full"sv;
    static constexpr auto FAILED_SV       = "FAILED"sv;

    enum SendStatus : std::uint8_t {
        NONE,
        SENDING,
        SUCCESS,
        FAIL_SENDING,
        FULL,
        FAILED
    };

    static const string_view& as_strv(SendStatus sstat) noexcept;

    SendStatus send_status;

    inline const string_view& send_status_sv() const noexcept {
        return as_strv(send_status);
    }
};
void from_json(const json& j, SendSmsResult& res);

struct DeleteSms {
    using named_parameter = std::map<std::string, json>;

    static constexpr auto query_str = "DeleteSMS";

    enum Flag { ALL, CONTACT, CONTENT };

    Flag del_flag;
    int  contact_id;
    int  sms_id;

    DeleteSms(int contact_id, bool all = false)
        : del_flag{all ? ALL : CONTACT}, contact_id{contact_id}, sms_id{-1} {}

    DeleteSms(int contact_id, int sms_id)
        : del_flag{CONTENT}, contact_id{contact_id}, sms_id{sms_id} {}

    operator named_parameter() const {
        switch (del_flag) {
            case CONTENT:
                return {{"DelFlag", del_flag},
                        {"ContactId", contact_id},
                        {"SMSId", sms_id}};
            default:
                return {
                    {"DelFlag", del_flag},
                    {"ContactId", contact_id},
                };
        }
    }
};
}  // namespace messages

#endif
