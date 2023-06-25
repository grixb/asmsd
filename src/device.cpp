#include "device.hpp"

namespace messages {

Device::Device(
    string hostname, int port, string base_path,
    seconds keepalive, seconds timeout, version ver
) : _connector(std::make_unique<HttpClientConnector>(
        std::move(hostname), port, std::move(base_path)
    )),
    _client{*_connector, ver},
    _keepalive{keepalive} {
    _connector->timeout(timeout);
    _last_alive = steady_clock::now() - timeout;
}

Device::Device(Device&& other) noexcept :
    _connector(std::move(other._connector)),
    _client(std::move(other._client)),
    _keepalive{other._keepalive},
    _last_alive{other._last_alive},
    _sys_inf(std::move(other._sys_inf)),
    _sys_status(std::move(other._sys_status)),
    _con_state(std::move(other._con_state)),
    _sms_state(std::move(other._sms_state)),
    _contacts(std::move(other._contacts)),
    _contents(std::move(other._contents)) {}


bool Device::ensure_alive() {
    std::lock_guard<std::mutex> lock(_mutex);
    try {
        const auto res = 
            _client.CallMethod<json>("0", "HeartBeat");
    } catch (const std::runtime_error& e) {
        const auto what = string_view{e.what()};
        if (what.starts_with("connection error: "))
            return false;
        else throw e;
    }
    _last_alive = steady_clock::now();
    return true;
}

bool Device::is_alive() {
    bool alive = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        alive = steady_clock::now() - _last_alive 
        <= _connector->timeout();
    }
    return alive ? true : ensure_alive();
}

// void Device::still_alive() {
//     std::lock_guard<std::mutex> lock(_mutex);
//     _last_alive = steady_clock::now();
// }

void Device::run_keepalive(NotAlive not_alive) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _is_running = true;
    }
    for (;;) {
        std::this_thread::sleep_for(_keepalive);
        if (!ensure_alive()) {
            if(!not_alive(*this)) {
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _is_running = false;
                }
                break;
            }
            else continue;
        } 
        if (!is_running()) break;        
    }
}

void Device::stop_keepalive() {
    std::lock_guard<std::mutex> lock(_mutex);
    _is_running = false;
}

void Device::wait_alive() {
    // const auto prev_to = _connector->timeout();
    // {
    //     std::lock_guard<std::mutex> lock(_mutex);
    //     _connector->timeout(_keepalive);
    // }
    while(!is_alive() && is_running()) {
        std::this_thread::sleep_for(_keepalive);
    }
    // {
    //     std::lock_guard<std::mutex> lock(_mutex);
    //     _connector->timeout(prev_to);
    // }
}

bool Device::is_running() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _is_running;
}

void Device::set_default_headers(staff::Headers hdrs) {
    std::lock_guard<std::mutex> lock(_mutex);
    _connector->set_default_headers(std::move(hdrs));
}

const SystemInfo& Device::system_info() {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sys_inf = std::make_unique<SystemInfo>(
            _client.CallMethod<json>("1", SystemInfo::query_str));
        //still_alive();
    }
    return *_sys_inf;
}

const SystemStatus& Device::system_status() {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sys_status = std::make_unique<SystemStatus>(
                _client.CallMethod<json>("2", SystemStatus::query_str));
        //still_alive();
    }
    return *_sys_status;
}

const ConnectionState& Device::connection_state() {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _con_state = std::make_unique<ConnectionState>(
                _client.CallMethod<json>("3", ConnectionState::query_str));
        //still_alive();
    }
    return *_con_state;
}

const SmsStorageState& Device::sms_storage_state() {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sms_state = std::make_unique<SmsStorageState>(
                _client.CallMethod<json>("4", SmsStorageState::query_str));
        //still_alive();
    }
    return *_sms_state;
}

const SmsContactList& Device::sms_contacts(size_t page) {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _contacts = _client.CallMethodNamed<SmsContactList>(
                "5", SmsContactList::query_str,
                Page(page > 0 ? page - 1 : 0));
        _last_alive = steady_clock::now();
        //still_alive();
    }
    return _contacts;
}

const SmsContentList& Device::sms_contents(int contact, size_t page) {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _contents = _client.CallMethodNamed<SmsContentList>(
                "6", SmsContentList::query_str,
                GetSmsContentList(contact, page > 0 ? page - 1 : 0));
        _last_alive = steady_clock::now();
        //still_alive();
    }
    return _contents;
}

void Device::delete_sms(int contact, int sms) {
    if (is_alive()) try {
        std::lock_guard<std::mutex> lock(_mutex);
        _client.CallMethodNamed<json>(
            "7", DeleteSms::query_str,
            sms > 0 ? DeleteSms(contact, sms) : DeleteSms(contact));
        //still_alive();
    } catch (const jsonrpccxx::JsonRpcException &je) {
        if (je.Code() != jsonrpccxx::internal_error &&
            je.Message() != WORKAROUND)
            throw je;
    }    
}

SendSmsResult Device::send_sms(vector<string> nums, string content) {
    if (is_alive()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _client.CallMethodNamed<json>(
            "8", SendSms::query_str,
            SendSms(std::move(nums), std::move(content)));

        auto res =
            _client.CallMethod<SendSmsResult>("9", SendSmsResult::query_str);

        if (res.send_status == SendSmsResult::SENDING) {
            while (res.send_status == SendSmsResult::SENDING) {
                std::this_thread::sleep_for(1s);
                res = _client.CallMethod<SendSmsResult>(
                    "10", SendSmsResult::query_str);
            }
        }
        //still_alive();
        return res;
    }
    return SendSmsResult{SendSmsResult::FAILED};
}

} // namespace messages