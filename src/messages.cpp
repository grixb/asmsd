#include "messages.hpp"

#include "nlohmann/json.hpp"

namespace messages {

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

SystemInfo::SystemInfo(const json&& j) { j.get_to(*this); }

const string_view& SystemStatus::as_strv(NetworkType nett) noexcept {
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

const string_view& SystemStatus::as_strv(ConnectionStatus cs) noexcept {
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

const string_view& SystemStatus::as_strv(SmsState smss) noexcept {
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

SystemStatus::SystemStatus(const json&& j) { j.get_to(*this); }

void from_json(const json& j, SmsStorageState& ss) {
    j.at("UnreadReport").get_to(ss.unread_report);
    j.at("LeftCount").get_to(ss.left_count);
    j.at("MaxCount").get_to(ss.max_count);
    j.at("TUseCount").get_to(ss.use_count);
    j.at("UnreadSMSCount").get_to(ss.unread_count);
}

// SmsStorageState::SmsStorageState(const json&& j) { j.get_to(*this); }

void from_json(const json& j, ConnectionState& cs) {
    j.at("ConnectionStatus").get_to(cs.connection_status);
    j.at("Conprofileerror").get_to(cs.conprof_error);
    j.at("ClearCode").get_to(cs.clear_code);
    j.at("mPdpRejectCount").get_to(cs.m_pdp_reject_count);
    j.at("IPv4Adrress").get_to(cs.ipv4_address);
    j.at("IPv6Adrress").get_to(cs.ipv6_address);
    j.at("Speed_Dl").get_to(cs.dl_speed);
    j.at("Speed_Ul").get_to(cs.ul_speed);
    j.at("DlRate").get_to(cs.dl_rate);
    j.at("UlRate").get_to(cs.ul_rate);
    size_t ctime{};
    j.at("ConnectionTime").get_to(ctime);
    cs.connection_time = seconds(ctime);
    j.at("UlBytes").get_to(cs.ul_bytes);
    j.at("DlBytes").get_to(cs.dl_bytes);
}

ConnectionState::ConnectionState(const json&& j) { j.get_to(*this); }

void from_json(const json& j, Page& p) { j.at("Page").get_to(p.page); }

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

void from_json(const json& j, SmsContent& smsc) {
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

void from_json(const json& j, SmsContentList& smsc) {
    j.get_to(static_cast<Pagination<SmsContentList>&>(smsc));
    j.at("ContactId").get_to(smsc.contact_id);
    j.at("PhoneNumber").get_to(smsc.phone_numbers);
    j.at("SMSContentList").get_to(smsc.contents);
}

void from_json(const json& j, SmsContact& smsc) {
    j.get_to(static_cast<SmsContent&>(smsc));
    j.at("ContactId").get_to(smsc.contact_id);
    j.at("PhoneNumber").get_to(smsc.phone_numbers);
    j.at("UnreadCount").get_to(smsc.unread_count);
    j.at("TSMSCount").get_to(smsc.sms_count);
}

void from_json(const json& j, SmsContactList& smsc) {
    j.get_to(static_cast<Pagination<SmsContactList>&>(smsc));
    j.at("SMSContactList").get_to(smsc.contacts);
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

void from_json(const json& j, SendSmsResult& res) {
    j.at("SendStatus").get_to(res.send_status);
}

} // namespace messages