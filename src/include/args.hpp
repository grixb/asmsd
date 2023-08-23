#ifndef ARGS_HPP
#define ARGS_HPP

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace spdlog::level {
enum level_enum : int;
}

namespace aux {
namespace fs = std::filesystem;

class Args {
    struct _Repr;
    std::unique_ptr<_Repr> _repr;

   public:
    template <typename T>
    using vector     = std::vector<T>;
    using seconds    = std::chrono::seconds;
    using minutes    = std::chrono::minutes;
    using string     = std::string;
    using level_enum = spdlog::level::level_enum;

    // vector<string>&& move_send_sms() & noexcept;
    // string&&         move_content() & noexcept;
    // string&&         move_base_path() & noexcept;
    // string&&         move_verify_token() & noexcept;
    // string&&         move_host() & noexcept;

    const level_enum&     log_level() const& noexcept;
    const string&         host() const& noexcept;
    string&&              host() && noexcept;
    const int&            port() const& noexcept;
    const string&         base_path() const& noexcept;
    string&&              base_path() && noexcept;
    const string&         verify_token() const& noexcept;
    string&&              verify_token() && noexcept;
    const bool&           detailed() const& noexcept;
    const bool&           system_info() const& noexcept;
    const bool&           system_status() const& noexcept;
    const bool&           connection_state() const& noexcept;
    const bool&           sms_storage_state() const& noexcept;
    const bool&           sms_contact_list() const& noexcept;
    const int&            sms_content_list() const& noexcept;
    const size_t&         page() const& noexcept;
    const vector<string>& send_sms() const& noexcept;
    vector<string>&&      send_sms() && noexcept;
    const string&         content() const& noexcept;
    string&&              content() && noexcept;
    const int&            delete_sms() const& noexcept;
    const fs::path&       watch() const& noexcept;
    const fs::path&       move_to() const& noexcept;
    const seconds&        time_out() const& noexcept;
    const seconds&        keep_alive() const& noexcept;
    const minutes&        reprocess() const& noexcept;

    Args();
    virtual ~Args();

    void parse(int argc, const char* argv[]);
};

}  // namespace aux

#endif