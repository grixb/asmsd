#include "httpclientconnector.hpp"

#include "messages.hpp"


namespace staff {

HttpClientConnector::HttpClientConnector(
    string hostname, int port, string base_path)
    : _client{hostname.c_str(), port}, 
    _timeout(seconds{15}), _path{base_path} {
    _hdrs = {{"Host", std::move(hostname)}, {"Connection", "keep-alive"}};
}

string HttpClientConnector::Send(const string& message) {
    _client.set_default_headers(_hdrs);
    _client.set_connection_timeout(_timeout);
    auto res = _client.Post(_path, message, "application/json");
    if (!res)
        throw std::runtime_error{
            fmt::format("connection error: {}", httplib::to_string(res.error()))
        };
    if (res->status != 200)
        throw jsonrpccxx::JsonRpcException(
            -32003, "http error: received status != 200");

    return res->body;
}

void HttpClientConnector::set_default_headers(Headers headers) {
    for (auto& [k, v] : headers) _hdrs.insert({std::move(k), std::move(v)});
}

void HttpClientConnector::timeout(const seconds& timeout) {
    _timeout = timeout;
}

}