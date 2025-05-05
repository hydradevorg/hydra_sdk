#ifndef HYDRA_SERVER_API_SERVER_HPP
#define HYDRA_SERVER_API_SERVER_HPP

#include <string>

namespace hydra {
namespace server {

class ApiServer {
public:
    ApiServer(int port, const std::string& api_key);
    ~ApiServer();

    void start();
    void stop();
    void set_api_key(const std::string& api_key);

private:
    class Impl;
    Impl* impl_;
};

} // namespace server
} // namespace hydra

#endif // HYDRA_SERVER_API_SERVER_HPP
