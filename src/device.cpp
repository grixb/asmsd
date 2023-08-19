#include "device.hpp"

#include <mutex>
#include <string_view>

#include "httpclientconnector.hpp"
#include "jsonrpccxx/client.hpp"
#include "messages_aux.hpp"

namespace {
using jsonrpccxx::JsonRpcClient;
using jsonrpccxx::version;
using std::chrono::steady_clock;
using time_point = steady_clock::time_point;

using namespace std::chrono_literals;
}  // namespace

namespace messages {

struct Device::_Impl {
    using Connector = staff::HttpClientConnector;

    Connector       _connector;
    version         _version;
    seconds         _keepalive;
    time_point      _last_alive;
    SystemInfo      _sys_inf;
    SystemStatus    _sys_status;
    ConnectionState _con_state;
    SmsStorageState _sms_state;
    SmsContactList  _contacts;
    SmsContentList  _contents;
    std::mutex      _mutex;
    bool            _is_running;

    _Impl(Connector con, version ver, seconds kal)
        : _connector(std::move(con)), _version(ver), _keepalive(kal) {}

    bool ensure_alive();
    bool is_alive();
};

Device::Device(string hostname, int port, string base_path, seconds keepalive,
               seconds timeout, char ver)
    : _impl(std::make_unique<_Impl>(
          _Impl::Connector(std::move(hostname), port, std::move(base_path)),
          ver == 1 ? version::v1 : version::v2, keepalive)) {
    _impl->_connector.timeout(timeout);
    _impl->_last_alive = steady_clock::now() - timeout;
}

Device::Device(Device&& other) noexcept {
    std::unique_lock l1{_impl->_mutex, std::defer_lock};
    std::unique_lock l2{other._impl->_mutex, std::defer_lock};
    std::lock(l1, l2);
    _impl                    = std::move(other._impl);
    other._impl->_is_running = false;
}

Device::~Device() = default;

bool Device::_Impl::ensure_alive() {
    try {
        auto       client = JsonRpcClient{_connector, _version};
        const auto res    = client.CallMethod<aux::json>("0", "HeartBeat");
    } catch (const std::runtime_error& e) {
        const auto what = string_view{e.what()};
        if (what.starts_with("connection error: "))
            return false;
        else
            throw e;
    }
    _last_alive = steady_clock::now();
    return true;
}

bool Device::_Impl::is_alive() {
    bool alive = false;
    alive      = steady_clock::now() - _last_alive <= _connector.timeout();

    return alive ? true : ensure_alive();
}

void Device::run_keepalive(NotAlive not_alive) {
    {
        std::lock_guard<std::mutex> lock(_impl->_mutex);
        _impl->_is_running = true;
    }
    for (;;) {
        std::this_thread::sleep_for(_impl->_keepalive);
        std::lock_guard<std::mutex> lock(_impl->_mutex);
        if (!_impl->ensure_alive()) {
            if (!not_alive(*this)) {
                _impl->_is_running = false;
                break;
            } else
                continue;
        }
        if (!_impl->_is_running) break;
    }
}

void Device::stop_keepalive() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);
    _impl->_is_running = false;
}

void Device::wait_alive() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);
    while (!_impl->is_alive() && _impl->_is_running) {
        std::this_thread::sleep_for(_impl->_keepalive);
    }
}

bool Device::is_running() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);
    return _impl->_is_running;
}

void Device::set_default_headers(HeadersInit hdrs) {
    std::lock_guard<std::mutex> lock(_impl->_mutex);
    _impl->_connector.set_default_headers(std::move(hdrs));
}

const SystemInfo& Device::system_info() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client = JsonRpcClient{_impl->_connector, _impl->_version};
        aux::emplace_json(
            _impl->_sys_inf,
            client.CallMethod<aux::json>("1", aux::query_str<SystemInfo>()));
    }
    return _impl->_sys_inf;
}

const SystemStatus& Device::system_status() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client = JsonRpcClient{_impl->_connector, _impl->_version};
        aux::emplace_json(
            _impl->_sys_status,
            client.CallMethod<aux::json>("2", aux::query_str<SystemStatus>()));
    }
    return _impl->_sys_status;
}

const ConnectionState& Device::connection_state() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client = JsonRpcClient{_impl->_connector, _impl->_version};
        aux::emplace_json(_impl->_con_state,
                          client.CallMethod<aux::json>(
                              "3", aux::query_str<ConnectionState>()));
    }
    return _impl->_con_state;
}

const SmsStorageState& Device::sms_storage_state() {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto       client = JsonRpcClient{_impl->_connector, _impl->_version};
        const auto repl   = client.CallMethod<aux::json>(
            "4", aux::query_str<SmsStorageState>());
        aux::from_json(repl, _impl->_sms_state);
    }
    return _impl->_sms_state;
}

const SmsContactList& Device::sms_contacts(size_t page) {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client      = JsonRpcClient{_impl->_connector, _impl->_version};
        _impl->_contacts = client.CallMethodNamed<SmsContactList>(
            "5", aux::query_str<SmsContactList>(),
            aux::as_param(Page(page > 0 ? page - 1 : 0)));
        _impl->_last_alive = steady_clock::now();
    }
    return _impl->_contacts;
}

const SmsContentList& Device::sms_contents(int contact, size_t page) {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client      = JsonRpcClient{_impl->_connector, _impl->_version};
        _impl->_contents = client.CallMethodNamed<SmsContentList>(
            "6", aux::query_str<SmsContentList>(),
            aux::as_param(GetSmsContentList(contact, page > 0 ? page - 1 : 0)));
        _impl->_last_alive = steady_clock::now();
    }
    return _impl->_contents;
}

static constexpr auto WORKAROUND =
    R"(invalid error response: "code" (negative number) and "message" (string) are required)";

void Device::delete_sms(int contact, int sms) {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) try {
            auto client = JsonRpcClient{_impl->_connector, _impl->_version};
            client.CallMethodNamed<aux::json>(
                "7", DeleteSms::query_str,
                aux::as_param(sms > 0 ? DeleteSms(contact, sms)
                                      : DeleteSms(contact)));
        } catch (const jsonrpccxx::JsonRpcException& je) {
            if (je.Code() != jsonrpccxx::internal_error &&
                je.Message() != WORKAROUND)
                throw je;
        }
}

SendSmsResult Device::send_sms(vector<string> nums, string content) {
    std::lock_guard<std::mutex> lock(_impl->_mutex);

    if (_impl->is_alive()) {
        auto client = JsonRpcClient{_impl->_connector, _impl->_version};
        client.CallMethodNamed<aux::json>(
            "8", aux::query_str<SendSms>(),
            aux::as_param(SendSms(std::move(nums), std::move(content))));

        auto res = client.CallMethod<SendSmsResult>(
            "9", aux::query_str<SendSmsResult>());

        if (res.send_status == SendSmsResult::SENDING) {
            while (res.send_status == SendSmsResult::SENDING) {
                std::this_thread::sleep_for(1s);
                res = client.CallMethod<SendSmsResult>(
                    "10", aux::query_str<SendSmsResult>());
            }
        }
        return res;
    }
    return SendSmsResult{SendSmsResult::FAILED};
}

}  // namespace messages