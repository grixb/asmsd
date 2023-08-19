#ifndef DEVICE_H
#define DEVICE_H

#include <chrono>
#include <functional>
#include <initializer_list>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "messages.hpp"

namespace messages {
using std::vector;
using std::chrono::seconds;

class Device {
    struct _Impl;
    std::unique_ptr<_Impl> _impl;

   public:
    using NotAlive = std::function<bool(Device&)>;
    using HeadersInit =
        std::initializer_list<std::pair<std::string, std::string>>;

    Device(string hostname, int port, string base_path, seconds keepalive,
           seconds timeout, char ver);

    Device(Device&& other) noexcept;

    virtual ~Device();

    const SystemInfo&      system_info();
    const SystemStatus&    system_status();
    const ConnectionState& connection_state();
    const SmsStorageState& sms_storage_state();
    const SmsContactList&  sms_contacts(size_t page);
    const SmsContentList&  sms_contents(int contact, size_t page);
    void                   delete_sms(int contact, int sms = -1);
    SendSmsResult          send_sms(vector<string> nums, string content);
    void                   run_keepalive(NotAlive not_alive);
    void                   stop_keepalive();
    void                   wait_alive();
    void                   set_default_headers(HeadersInit hdrs);
    bool                   is_running();
};
}  // namespace messages

#endif