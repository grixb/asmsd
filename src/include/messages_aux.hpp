#ifndef MESSAGES_AUX_HPP
#define MESSAGES_AUX_HPP
#include "messages.hpp"
#include "nlohmann/json.hpp"
#include <concepts>

namespace messages {
struct aux {
    using json            = nlohmann::json;
    using named_parameter = std::map<std::string, json>;

    static SystemInfo& emplace_json(SystemInfo&, json&&);
    static SystemStatus& emplace_json(SystemStatus&, json&&);
    static SmsStorageState& from_json(const json&, SmsStorageState&);
    static ConnectionState& emplace_json(ConnectionState&, json&&);

    static named_parameter as_param(const Page&);
    static named_parameter as_param(const GetSmsContentList&);
    static named_parameter as_param(SendSms&&);
    static named_parameter as_param(const DeleteSms&);

    template<typename Q>
    struct query_str_tag{};
    static const char* query_str_(query_str_tag<SystemInfo>) noexcept;
    static const char* query_str_(query_str_tag<SystemStatus>) noexcept;
    static const char* query_str_(query_str_tag<SmsStorageState>) noexcept;
    static const char* query_str_(query_str_tag<ConnectionState>) noexcept;
    static const char* query_str_(query_str_tag<SmsContentList>) noexcept;
    static const char* query_str_(query_str_tag<SmsContactList>) noexcept;
    static const char* query_str_(query_str_tag<SendSms>) noexcept;
    static const char* query_str_(query_str_tag<SendSmsResult>) noexcept;

    template<typename Q>
    static const char* query_str() noexcept {
        return query_str_(query_str_tag<Q>{});
}
};

template <typename CRTP>
void from_json(const aux::json& j, Pagination<CRTP>& p) {
    //j.get_to(static_cast<Page&>(p));
    j.at("Page").get_to(p.page);
    j.at("TotalPageCount").get_to(p.total_pages);
}

void from_json(const aux::json& j, SmsContent&);
void from_json(const aux::json& j, SmsContentList&);
void from_json(const aux::json& j, SmsContact&);
void from_json(const aux::json& j, SmsContactList&);
void from_json(const aux::json& j, SendSmsResult&);

}
#endif // MESSAGES_AUX_HPP