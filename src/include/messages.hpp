#ifndef MESSAGES_H
#define MESSAGES_H

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace messages {
using namespace std::string_view_literals;

using std::string;
using std::string_view;
using std::time_t;
using std::chrono::seconds;
using std::vector;

class SystemInfo {
    class _Repr; std::unique_ptr<_Repr> _repr;

public:
    using mac_addr_t = std::uint8_t[6];

    friend struct aux;

    const string_view& device_name() const& noexcept;
    const string_view& hw_version() const& noexcept;
    const std::tm&     build_time() const& noexcept;
    const string_view& http_api_version() const& noexcept;
    const string_view& iccid() const& noexcept;
    const string_view& imei() const& noexcept;
    const int&         imeisv() const& noexcept;
    const string_view& imsi() const& noexcept;
    const mac_addr_t&  mac_address() const& noexcept;

    SystemInfo();
    SystemInfo(SystemInfo&&) noexcept;
    SystemInfo& operator=(SystemInfo&&) noexcept;

    virtual ~SystemInfo();
};

static constexpr auto UNKNOWN = "UNKNOWN"sv;

class SystemStatus {
    class _Repr; std::unique_ptr<_Repr> _repr;

public:
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

    enum class NetworkType : std::uint8_t {
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

    enum class ConnectionStatus : std::uint8_t {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

    enum class SmsState : std::uint8_t { DISABLED, FULL, NORMAL, NEW };

    friend struct aux;

    const string_view&      network_name() const& noexcept;
    const int&              signal_strength() const& noexcept;
    const int&              conprof_error() const& noexcept;
    const int&              clear_code() const& noexcept;
    const int&              m_pdp_reject_count() const& noexcept;
    const NetworkType&      network_type() const& noexcept;
    const ConnectionStatus& connection_status() const& noexcept;
    const SmsState&         sms_state() const& noexcept;
    const bool&             roaming() const& noexcept;
    const bool&             domestic_roaming() const& noexcept;

    static const string_view& as_strv(NetworkType) noexcept;
    static const string_view& as_strv(ConnectionStatus) noexcept;
    static const string_view& as_strv(SmsState) noexcept;

    inline const string_view& network_type_sv() const& {
        return as_strv(network_type());
    }
    inline const string_view& connection_status_sv() const& {
        return as_strv(connection_status());
    }
    inline const string_view& sms_state_sv() const& {
        return as_strv(sms_state());
    }

    SystemStatus();
    SystemStatus(SystemStatus&&) noexcept;
    SystemStatus& operator=(SystemStatus&&) noexcept;

    virtual ~SystemStatus();
};

class SmsStorageState {
    class _Repr; std::unique_ptr<_Repr> _repr;

public:
    friend struct aux;

    const int& unread_report() const& noexcept;
    const int& left_count() const& noexcept;
    const int& max_count() const& noexcept;
    const int& use_count() const& noexcept;
    const int& unread_count() const& noexcept;

    SmsStorageState();
    SmsStorageState(SmsStorageState&&) noexcept;
    SmsStorageState& operator=(SmsStorageState&&) noexcept;

    virtual ~SmsStorageState();    
};

using ConnectionStatus = SystemStatus::ConnectionStatus;

class ConnectionState {
    class _Repr; std::unique_ptr<_Repr> _repr;

public:
    friend struct aux;

    const ConnectionStatus& connection_status() const& noexcept;
    const int&              conprof_error() const& noexcept;
    const int&              clear_code() const& noexcept;
    const int&              m_pdp_reject_count() const& noexcept;
    const string_view&      ipv4_address() const& noexcept;
    const string_view&      ipv6_address() const& noexcept;
    const size_t&           dl_speed() const& noexcept;
    const size_t&           ul_speed() const& noexcept;
    const size_t&           dl_rate() const& noexcept;
    const size_t&           ul_rate() const& noexcept;
    const size_t&           dl_bytes() const& noexcept;
    const size_t&           ul_bytes() const& noexcept;
    const seconds&          connection_time() const& noexcept;

    inline const string_view& connection_status_sv() const noexcept {
        return SystemStatus::as_strv(connection_status());
    }

    ConnectionState();
    ConnectionState(ConnectionState&&) noexcept;
    ConnectionState& operator=(ConnectionState&&) noexcept;

    virtual ~ConnectionState();
};

struct Page {
    size_t page;
    Page(size_t p) : page{p} {}
    Page() = default;
};

template <typename CRTP>
struct Pagination : Page {
    using crtp_t = CRTP;
    int total_pages;
};
struct GetSmsContentList : Page {
    int contact_id;
    GetSmsContentList(int id, size_t p) : Page{p}, contact_id{id} {}
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

struct SmsContentList : Pagination<SmsContentList> {
    vector<SmsContent> contents;
    int                contact_id;
    vector<string>     phone_numbers;
};

struct SmsContact : SmsContent {
    int            contact_id;
    vector<string> phone_numbers;
    size_t         unread_count;
    int            sms_count;
};

struct SmsContactList : Pagination<SmsContactList> {
    vector<SmsContact> contacts;
};

struct SendSms {
    using clock           = std::chrono::system_clock;

    int            sms_id;
    string         sms_content;
    vector<string> phone_numbers;
    time_t         sms_time;

    SendSms(vector<string> nums, string content);
};

struct SendSmsResult {
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

    static const string_view& as_strv(SendStatus) noexcept;

    SendStatus send_status;

    inline const string_view& send_status_sv() const noexcept {
        return as_strv(send_status);
    }
};


struct DeleteSms {
    static constexpr auto query_str = "DeleteSMS";

    enum Flag { ALL, CONTACT, CONTENT };

    Flag del_flag;
    int  contact_id;
    int  sms_id;

    DeleteSms(int contact_id, bool all = false)
        : del_flag{all ? ALL : CONTACT}, contact_id{contact_id}, sms_id{-1} {}

    DeleteSms(int contact_id, int sms_id)
        : del_flag{CONTENT}, contact_id{contact_id}, sms_id{sms_id} {}
};
}  // namespace messages

#endif
