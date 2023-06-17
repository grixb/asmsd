#ifndef HTTP_CLIENT_CONNECTOR_H
#define HTTP_CLIENT_CONNECTOR_H

#ifndef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
#define CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
#endif

#include <string>

#include "httplib.h"
#include "jsonrpccxx/common.hpp"
#include "jsonrpccxx/iclientconnector.hpp"
//#include <string_view>

namespace staff {
using httplib::Headers;
using jsonrpccxx::IClientConnector;
using jsonrpccxx::JsonRpcException;
using std::string;
using std::string_view;

class HttpClientConnector : public IClientConnector {
    httplib::Client _client;
    string          _path;
    Headers         _hdrs;

   public:
    explicit HttpClientConnector(string hostname, int port, string base_path)
        : _client{hostname.c_str(), port}, _path{base_path} {
        _hdrs = {{"Host", std::move(hostname)}, {"Connection", "keep-alive"}};
    }

    string Send(const string& message) override {
        _client.set_default_headers(_hdrs);
        auto res = _client.Post(_path, message, "application/json");
        if (!res || res->status != 200)
            throw jsonrpccxx::JsonRpcException(
                -32003, "client connector error, received status != 200");

        return res->body;
    }

    void set_default_headers(Headers headers) {
        for (auto& [k, v] : headers) _hdrs.insert({std::move(k), std::move(v)});
    }
};
}  // end namespace staff

#endif