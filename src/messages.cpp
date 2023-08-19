#include <sstream>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "messages_aux.hpp"
#include "nlohmann/json.hpp"

namespace messages {

class SystemInfo::_Repr {
    friend class SystemInfo;
    friend struct aux;

    aux::json   _j_obj;
    string_view _device_name;
    string_view _hw_version;
    string_view _http_api_version;
    string_view _iccid;
    string_view _imei;
    string_view _imsi;
    std::tm     _build_time;
    mac_addr_t  _mac_addr;
    string_view _imeisv;

    void refresh() {
        _j_obj.at("DeviceName").get_to(_device_name);
        _j_obj.at("HwVersion").get_to(_hw_version);
        _j_obj.at("HttpApiVersion").get_to(_http_api_version);
        _j_obj.at("ICCID").get_to(_iccid);
        _j_obj.at("IMEI").get_to(_imei);
        _j_obj.at("IMEISV").get_to(_imeisv);
        _j_obj.at("IMSI").get_to(_imsi);
        const auto& tmp = _j_obj.at("BuildTime").get_ref<const string&>();
        std::istringstream{tmp} >>
            std::get_time(&_build_time, "%Y-%m-%d %H:%M:%S");
        const auto& str = _j_obj.at("MacAddress").get_ref<const string&>();
        for (size_t j{}; j < 6; ++j)
            _mac_addr[j] = std::stoul(str.substr(j * 3, 2), nullptr, 16);
    }

    _Repr(aux::json&& j) noexcept : _j_obj(std::move(j)) {}
};

const char* aux::query_str_(query_str_tag<SystemInfo>) noexcept {
    return "GetSystemInfo";
}

SystemInfo& aux::emplace_json(SystemInfo& i, json&& j) {
    if (i._repr)
        i._repr->_j_obj = std::move(j);
    else
        i._repr = std::unique_ptr<SystemInfo::_Repr>(
            new SystemInfo::_Repr(std::move(j)));
    i._repr->refresh();
    return i;
}

SystemInfo::SystemInfo()                                 = default;
SystemInfo::SystemInfo(SystemInfo&&) noexcept            = default;
SystemInfo& SystemInfo::operator=(SystemInfo&&) noexcept = default;

SystemInfo::~SystemInfo() = default;

const string_view& SystemInfo::device_name() const& noexcept {
    return _repr->_device_name;
}

const string_view& SystemInfo::hw_version() const& noexcept {
    return _repr->_hw_version;
}

const tm& SystemInfo::build_time() const& noexcept {
    return _repr->_build_time;
}

const string_view& SystemInfo::http_api_version() const& noexcept {
    return _repr->_http_api_version;
}

const string_view& SystemInfo::iccid() const& noexcept { return _repr->_iccid; }

const string_view& SystemInfo::imei() const& noexcept { return _repr->_imei; }

const string_view& SystemInfo::imeisv() const& noexcept {
    return _repr->_imeisv;
}

const string_view& SystemInfo::imsi() const& noexcept { return _repr->_imsi; }

const SystemInfo::mac_addr_t& SystemInfo::mac_address() const& noexcept {
    return _repr->_mac_addr;
}

const string_view& SystemStatus::as_strv(NetworkType nett) noexcept {
    switch (nett) {
        case NetworkType::NO_SERVICE:
            return NO_SERVICE_SV;
        case NetworkType::GPRS:
            return GPRS_SV;
        case NetworkType::EDGE:
            return EDGE_SV;
        case NetworkType::HSPA:
            return HSPA_SV;
        case NetworkType::HSUPA:
            return HSUPA_SV;
        case NetworkType::UMTS:
            return UMTS_SV;
        case NetworkType::HSPA_PLUS:
            return HSPAP_SV;
        case NetworkType::DC_HSPA_PLUS:
            return DCHSPAP_SV;
        case NetworkType::LTE:
            return LTE_SV;
        case NetworkType::LTE_PLUS:
            return LTEP_SV;
        default:
            return UNKNOWN;
    }
}

const string_view& SystemStatus::as_strv(ConnectionStatus cs) noexcept {
    switch (cs) {
        case ConnectionStatus::DISCONNECTED:
            return DISCONNECTED_SV;
        case ConnectionStatus::CONNECTING:
            return CONNECTED_SV;
        case ConnectionStatus::CONNECTED:
            return CONNECTED_SV;
        case ConnectionStatus::DISCONNECTING:
            return DISCONNECTING_SV;
        default:
            return UNKNOWN;
    }
}

const string_view& SystemStatus::as_strv(SmsState smss) noexcept {
    switch (smss) {
        case SmsState::DISABLED:
            return DISABLED_SV;
        case SmsState::FULL:
            return FULL_SV;
        case SmsState::NORMAL:
            return NORMAL_SV;
        case SmsState::NEW:
            return NEW_SV;
        default:
            return UNKNOWN;
    }
}

class SystemStatus::_Repr {
    friend class SystemStatus;
    friend struct aux;

    aux::json        _j_obj;
    string_view      _network_name;
    int              _signal_strength;
    int              _conprof_error;
    int              _clear_code;
    int              _m_pdp_reject_count;
    NetworkType      _network_type;
    ConnectionStatus _connection_status;
    SmsState         _sms_state;
    bool             _roaming;
    bool             _domestic_roaming;

    void refresh() {
        _j_obj.at("NetworkName").get_to(_network_name);
        _j_obj.at("SignalStrength").get_to(_signal_strength);
        _j_obj.at("Conprofileerror").get_to(_conprof_error);
        _j_obj.at("ClearCode").get_to(_clear_code);
        _j_obj.at("mPdpRejectCount").get_to(_m_pdp_reject_count);
        _j_obj.at("NetworkType").get_to(_network_type);
        _j_obj.at("ConnectionStatus").get_to(_connection_status);
        _j_obj.at("SmsState").get_to(_sms_state);
        int tmp;
        _j_obj.at("Roaming").get_to(tmp);
        _roaming = tmp > 0;
        _j_obj.at("Domestic_Roaming").get_to(tmp);
        _domestic_roaming = tmp > 0;
    }

    _Repr(aux::json&& j) noexcept : _j_obj(std::move(j)) {}
};

const char* aux::query_str_(query_str_tag<SystemStatus>) noexcept {
    return "GetSystemStatus";
}

SystemStatus& aux::emplace_json(SystemStatus& s, json&& j) {
    if (s._repr)
        s._repr->_j_obj = std::move(j);
    else
        s._repr = std::unique_ptr<SystemStatus::_Repr>(
            new SystemStatus::_Repr(std::move(j)));
    s._repr->refresh();
    return s;
}

SystemStatus::SystemStatus()                                   = default;
SystemStatus::SystemStatus(SystemStatus&&) noexcept            = default;
SystemStatus& SystemStatus::operator=(SystemStatus&&) noexcept = default;
SystemStatus::~SystemStatus()                                  = default;

const string_view& SystemStatus::network_name() const& noexcept {
    return _repr->_network_name;
}

const int& SystemStatus::signal_strength() const& noexcept {
    return _repr->_signal_strength;
}

const int& SystemStatus::conprof_error() const& noexcept {
    return _repr->_conprof_error;
}

const int& SystemStatus::clear_code() const& noexcept {
    return _repr->_clear_code;
}

const int& SystemStatus::m_pdp_reject_count() const& noexcept {
    return _repr->_m_pdp_reject_count;
}

const SystemStatus::NetworkType& SystemStatus::network_type() const& noexcept {
    return _repr->_network_type;
}

const SystemStatus::ConnectionStatus& SystemStatus::connection_status()
    const& noexcept {
    return _repr->_connection_status;
}

const SystemStatus::SmsState& SystemStatus::sms_state() const& noexcept {
    return _repr->_sms_state;
}

const bool& SystemStatus::roaming() const& noexcept { return _repr->_roaming; }

const bool& SystemStatus::domestic_roaming() const& noexcept {
    return _repr->_domestic_roaming;
}

class SmsStorageState::_Repr {
    friend class SmsStorageState;
    friend struct aux;

    int unread_report;
    int left_count;
    int max_count;
    int use_count;
    int unread_count;
};

const char* aux::query_str_(query_str_tag<SmsStorageState>) noexcept {
    return "GetSMSStorageState";
}

SmsStorageState& aux::from_json(const json& j, SmsStorageState& ss) {
    if (!ss._repr) ss._repr = std::make_unique<SmsStorageState::_Repr>();
    j.at("UnreadReport").get_to(ss._repr->unread_count);
    j.at("LeftCount").get_to(ss._repr->left_count);
    j.at("MaxCount").get_to(ss._repr->max_count);
    j.at("TUseCount").get_to(ss._repr->use_count);
    j.at("UnreadSMSCount").get_to(ss._repr->unread_count);
    return ss;
}

SmsStorageState::SmsStorageState()                           = default;
SmsStorageState::SmsStorageState(SmsStorageState&&) noexcept = default;
SmsStorageState& SmsStorageState::operator=(SmsStorageState&&) noexcept =
    default;

SmsStorageState::~SmsStorageState() = default;

const int& SmsStorageState::unread_report() const& noexcept {
    return _repr->unread_report;
}
const int& SmsStorageState::left_count() const& noexcept {
    return _repr->left_count;
}
const int& SmsStorageState::max_count() const& noexcept {
    return _repr->max_count;
}
const int& SmsStorageState::use_count() const& noexcept {
    return _repr->use_count;
}
const int& SmsStorageState::unread_count() const& noexcept {
    return _repr->unread_count;
}

class ConnectionState::_Repr {
    friend class ConnectionState;
    friend struct aux;

    aux::json        _j_obj;
    ConnectionStatus connection_status;
    int              conprof_error;
    int              clear_code;
    int              m_pdp_reject_count;
    string_view      ipv4_address;
    string_view      ipv6_address;
    size_t           dl_speed;
    size_t           ul_speed;
    size_t           dl_rate;
    size_t           ul_rate;
    size_t           dl_bytes;
    size_t           ul_bytes;
    seconds          connection_time;

    _Repr(aux::json&& j) noexcept : _j_obj(std::move(j)) {}

    void refresh() {
        _j_obj.at("ConnectionStatus").get_to(connection_status);
        _j_obj.at("Conprofileerror").get_to(conprof_error);
        _j_obj.at("ClearCode").get_to(clear_code);
        _j_obj.at("mPdpRejectCount").get_to(m_pdp_reject_count);
        _j_obj.at("IPv4Adrress").get_to(ipv4_address);
        _j_obj.at("IPv6Adrress").get_to(ipv6_address);
        _j_obj.at("Speed_Dl").get_to(dl_speed);
        _j_obj.at("Speed_Ul").get_to(ul_speed);
        _j_obj.at("DlRate").get_to(dl_rate);
        _j_obj.at("UlRate").get_to(ul_rate);
        size_t ctime{};
        _j_obj.at("ConnectionTime").get_to(ctime);
        connection_time = seconds(ctime);
        _j_obj.at("UlBytes").get_to(ul_bytes);
        _j_obj.at("DlBytes").get_to(dl_bytes);
    }
};

const char* aux::query_str_(query_str_tag<ConnectionState>) noexcept {
    return "GetConnectionState";
}

ConnectionState& aux::emplace_json(ConnectionState& s, json&& j) {
    if (s._repr)
        s._repr->_j_obj = std::move(j);
    else
        s._repr = std::unique_ptr<ConnectionState::_Repr>(
            new ConnectionState::_Repr(std::move(j)));
    s._repr->refresh();
    return s;
}

ConnectionState::ConnectionState()                           = default;
ConnectionState::ConnectionState(ConnectionState&&) noexcept = default;
ConnectionState& ConnectionState::operator=(ConnectionState&&) noexcept =
    default;

ConnectionState::~ConnectionState() = default;

const string_view& SmsContent::as_strv(SmsType smst) noexcept {
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

const ConnectionStatus& ConnectionState::connection_status() const& noexcept {
    return _repr->connection_status;
}
const int& ConnectionState::conprof_error() const& noexcept {
    return _repr->conprof_error;
}
const int& ConnectionState::clear_code() const& noexcept {
    return _repr->clear_code;
}
const int& ConnectionState::m_pdp_reject_count() const& noexcept {
    return _repr->m_pdp_reject_count;
}
const string_view& ConnectionState::ipv4_address() const& noexcept {
    return _repr->ipv4_address;
}
const string_view& ConnectionState::ipv6_address() const& noexcept {
    return _repr->ipv6_address;
}
const size_t& ConnectionState::dl_speed() const& noexcept {
    return _repr->dl_speed;
}
const size_t& ConnectionState::ul_speed() const& noexcept {
    return _repr->ul_speed;
}
const size_t& ConnectionState::dl_rate() const& noexcept {
    return _repr->dl_rate;
}
const size_t& ConnectionState::ul_rate() const& noexcept {
    return _repr->ul_rate;
}
const size_t& ConnectionState::dl_bytes() const& noexcept {
    return _repr->dl_bytes;
}
const size_t& ConnectionState::ul_bytes() const& noexcept {
    return _repr->ul_bytes;
}
const seconds& ConnectionState::connection_time() const& noexcept {
    return _repr->connection_time;
}

aux::named_parameter aux::as_param(const Page& p) { return {{"Page", p.page}}; }

aux::named_parameter aux::as_param(const GetSmsContentList& g) {
    return {{"Page", g.page}, {"ContactId", g.contact_id}};
}

void from_json(const aux::json& j, SmsContent& smsc) {
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
}

void from_json(const aux::json& j, SmsContentList& smsc) {
    j.get_to(static_cast<Pagination<SmsContentList>&>(smsc));
    j.at("ContactId").get_to(smsc.contact_id);
    j.at("PhoneNumber").get_to(smsc.phone_numbers);
    j.at("SMSContentList").get_to(smsc.contents);
}

const char* aux::query_str_(query_str_tag<SmsContentList>) noexcept {
    return "GetSMSContentList";
}

void from_json(const aux::json& j, SmsContact& smsc) {
    j.get_to(static_cast<SmsContent&>(smsc));
    j.at("ContactId").get_to(smsc.contact_id);
    j.at("PhoneNumber").get_to(smsc.phone_numbers);
    j.at("UnreadCount").get_to(smsc.unread_count);
    j.at("TSMSCount").get_to(smsc.sms_count);
}

void from_json(const aux::json& j, SmsContactList& smsc) {
    j.get_to(static_cast<Pagination<SmsContactList>&>(smsc));
    j.at("SMSContactList").get_to(smsc.contacts);
}

const char* aux::query_str_(query_str_tag<SmsContactList>) noexcept {
    return "GetSMSContactList";
}

const string_view& SendSmsResult::as_strv(SendStatus sstat) noexcept {
    switch (sstat) {
        case NONE:
            return NONE_SV;
        case SENDING:
            return SENDING_SV;
        case SUCCESS:
            return SUCCESS_SV;
        case FAIL_SENDING:
            return FAIL_SENDING_SV;
        case FULL:
            return FULL_SV;
        case FAILED:
            return FAILED_SV;
        default:
            return UNKNOWN;
    }
}

void from_json(const aux::json& j, SendSmsResult& res) {
    j.at("SendStatus").get_to(res.send_status);
}

aux::named_parameter aux::as_param(SendSms&& sms) {
    return {{"SMSId", sms.sms_id},
            {"SMSContent", std::move(sms.sms_content)},
            {"PhoneNumber", std::move(sms.phone_numbers)},
            {"SMSTime", fmt::format("{:%Y-%m-%d %H:%M:%S}",
                                    *std::localtime(&sms.sms_time))}};
}

SendSms::SendSms(vector<string> nums, string content)
    : sms_id{-1},
      sms_content{std::move(content)},
      phone_numbers{std::move(nums)} {
    sms_time = clock::to_time_t(clock::now());
}

const char* aux::query_str_(query_str_tag<SendSms>) noexcept {
    return "SendSMS";
}

const char* aux::query_str_(query_str_tag<SendSmsResult>) noexcept {
    return "GetSendSMSResult";
}

aux::named_parameter aux::as_param(const DeleteSms& ds) {
    switch (ds.del_flag) {
        case DeleteSms::CONTENT:
            return {{"DelFlag", ds.del_flag},
                    {"ContactId", ds.contact_id},
                    {"SMSId", ds.sms_id}};
        default:
            return {
                {"DelFlag", ds.del_flag},
                {"ContactId", ds.contact_id},
            };
    }
}

}  // namespace messages