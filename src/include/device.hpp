#ifndef DEVICE_H
#define DEVICE_H

#include "httpclientconnector.hpp"
#include "messages.hpp"

#include "jsonrpccxx/client.hpp"

#include <memory>
#include <chrono>
#include <exception>
#include <vector>
#include <string_view>
#include <mutex>
#include <functional>
#include <thread>

namespace messages {
using namespace std::chrono_literals;

using jsonrpccxx::JsonRpcClient;
using jsonrpccxx::version;
using std::chrono::seconds;
using std::vector;
using staff::HttpClientConnector;
using std::chrono::steady_clock;

using time_point = steady_clock::time_point;

template<typename T>
using uni = std::unique_ptr<T>;

static constexpr auto WORKAROUND =
    R"(invalid error response: "code" (negative number) and "message" (string) are required)";

class Device {
    uni<HttpClientConnector> _connector;
    
    version          _version;
    seconds          _keepalive;
    time_point       _last_alive;
    SystemInfo       _sys_inf;
    SystemStatus     _sys_status;
    ConnectionState  _con_state;
    SmsStorageState  _sms_state;
    SmsContactList   _contacts;
    SmsContentList   _contents;
    std::mutex       _mutex;
    bool             _is_running;


    bool ensure_alive();
    bool is_alive();
    // void still_alive();

public:

    using NotAlive = std::function<bool(Device&)>;

    Device(
        string hostname, int port, string base_path,
        seconds keepalive, seconds timeout, version ver
    );

    Device(Device&& other) noexcept;

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
    void                   set_default_headers(staff::Headers hdrs);
    bool                   is_running();
};
} // namespace messages

#endif