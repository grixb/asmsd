#ifndef HTTP_CLIENT_CONNECTOR_H
#define HTTP_CLIENT_CONNECTOR_H

#ifndef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
#define CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
#endif

#include <string>
#include <chrono>
#include <stdexcept>

#include "fmt/core.h"
#include "httplib.h"
#include "jsonrpccxx/common.hpp"
#include "jsonrpccxx/iclientconnector.hpp"

namespace staff {
using httplib::Headers;
using jsonrpccxx::IClientConnector;
using jsonrpccxx::JsonRpcException;
using std::string;
using std::string_view;
using std::chrono::seconds;

class HttpClientConnector : public IClientConnector {
    httplib::Client _client;
    seconds         _timeout;
    string          _path;
    Headers         _hdrs;

   public:
    explicit HttpClientConnector(
        string hostname, int port, string base_path)
        : _client{hostname.c_str(), port}, 
        _timeout(seconds{15}), _path{base_path} {
        _hdrs = {{"Host", std::move(hostname)}, {"Connection", "keep-alive"}};
    }

    string Send(const string& message) override {
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

    void set_default_headers(Headers headers) {
        for (auto& [k, v] : headers) _hdrs.insert({std::move(k), std::move(v)});
    }

    void timeout(const seconds& timeout) {
        _timeout = timeout;
    }

    inline const seconds& timeout() { return _timeout; }
};
}  // end namespace staff

#endif