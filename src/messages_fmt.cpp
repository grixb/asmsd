#include "messages_fmt.hpp"

#include "fmt/chrono.h"
#include "fmt/core.h"

namespace fmt {

template<>
auto formatter<SystemInfo>::format(const SystemInfo& info, format_context& ctx) const
    -> format_context::iterator {
    const auto& mac = info.mac_address;

    if (presentation == COMPACT)
        return format_to(
            ctx.out(),
            "SYS: {} @ {:%Y.%m.%d}, MAC: {:X}:{:X}:{:X}:{:X}:{:X}:{:X}, "
            "API: {}",
            info.hw_version, info.build_time_ltm(),          //
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],  //
            info.http_api_version);

    if (presentation == DETAILED)
        return fmt::format_to(
            ctx.out(),
            "SYSTEM INFO [{}]:\n"
            "\tHW Version:       {}\n"
            "\tMAC Address:      {:X}:{:X}:{:X}:{:X}:{:X}:{:X}\n"
            "\tBuild Time:       {:%Y.%m.%d. %H:%M}\n"
            "\tHTTP API version: {}\n"
            "\tICCID:            {}\n"
            "\tIMEI:             {}\n"
            "\tIMEI sv:          {}\n"
            "\tIMSI:             {}\n",
            info.device_name, info.hw_version,               //
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],  //
            info.build_time_ltm(),                           //
            info.http_api_version, info.iccid, info.imei, info.imeisv,
            info.imsi);

    return ctx.out();
}

template<>
auto formatter<SystemStatus>::format(const SystemStatus& status, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT)
        return format_to(
            ctx.out(),
            "RAN: {} @ {} is {} with signal strength {}, SMS: {}",
            status.network_type_sv(), status.network_name,
            status.connection_status_sv(), status.signal_strength,
            status.sms_state_sv());

    if (presentation == DETAILED)
        return format_to(ctx.out(),
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

template<>
auto formatter<SmsStorageState>::format(const SmsStorageState& smss, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT)
        return format_to(
            ctx.out(),
            "SMS Storage counters: Use: {}, unread/left/max: {}/{}/{}",
            smss.use_count, smss.unread_count, smss.left_count,
            smss.max_count);
    return ctx.out();
}

template<>
auto formatter<ConnectionState>::format(const ConnectionState& cs, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT)
        return format_to(
            ctx.out(),
            "{} {:%H:%M:%S} ago @ v4: {} v6: {}, traffic (UP/DOWN): {}/{}",
            cs.connection_status_sv(), cs.connection_time, cs.ipv4_address,
            cs.ipv6_address, cs.ul_bytes, cs.dl_bytes);

    if (presentation == DETAILED)
        return format_to(ctx.out(),
                                "CONNECTION STATE [{}]:\n"
                                "\tConnection Time:  {:%H:%M:%S}\n"
                                "\tConnection Error: {}\n"
                                "\tClear Code:       {}\n"
                                "\tIPv4 address:     {}\n"
                                "\tIPv6 address:     {}\n"
                                "\tDownload Speed:   {}\n"
                                "\tUpload Speed:     {}\n"
                                "\tDownloaded Bytes: {}\n"
                                "\tUploaded Bytes:   {}\n",
                                cs.connection_status_sv(), cs.connection_time,
                                cs.conprof_error, cs.clear_code,
                                cs.ipv4_address, cs.ipv6_address, cs.dl_speed,
                                cs.ul_speed, cs.dl_bytes, cs.ul_bytes);

    return ctx.out();
}

template<>
auto formatter<SmsContent>::format(const SmsContent& cntc, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT)
        return format_to(
            ctx.out(), "{} <{}> @ {:%Y.%m.%d. %H:%M}\n\t{}\n",
            cntc.sms_type_sv(), cntc.sms_id, cntc.sms_time_ltm(),
            cntc.sms_type == REPORT
                ? cntc.report_status ? "DELIVERED" : "FAILED"
                : cntc.sms_content);

    return ctx.out();
}

template<>
auto formatter<SmsContentList>::format(const SmsContentList& clst, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT) {
        auto out = fmt::format_to(
            ctx.out(), "SMS CONTENT LIST of <{}> {} [{}/{}]:\n",
            clst.contact_id, fmt::join(clst.phone_numbers, ", "),
            clst.page + 1, clst.total_pages);

        for (const SmsContent& content : clst.contents) {
            out = fmt::format_to(out, " {}\n", content);
        }

        return out;
    }

    return ctx.out();
}

template<>
auto formatter<SmsContact>::format(const SmsContact& cntc, format_context& ctx) const
    -> format_context::iterator {
    if (presentation == COMPACT)
        return fmt::format_to(ctx.out(), "<{}> {} [{}] {}", cntc.contact_id,
                                fmt::join(cntc.phone_numbers, ", "),
                                cntc.sms_count,
                                static_cast<const SmsContent&>(cntc));

    return ctx.out();
}

template<>
auto formatter<SmsContactList>::format(const SmsContactList& clst, format_context& ctx) const
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

}