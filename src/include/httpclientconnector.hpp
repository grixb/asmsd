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
        string hostname, int port, string base_path);

    string Send(const string& message) override;

    void set_default_headers(Headers headers);

    void timeout(const seconds& timeout);

    inline const seconds& timeout() { return _timeout; }
};
}  // end namespace staff

#endif