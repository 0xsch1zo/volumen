#include <misc/ssave.hpp>
#include <keychain/keychain.h>
#include <stdexcept>

namespace kc = keychain;

void ssave::save(const std::string& secret, const std::string& service) {
    kc::Error error;
    kc::setPassword(package_, service, user_, secret, error);
    if(error)
        throw std::runtime_error(error.message);
}

std::string ssave::get(const std::string& service) {
    kc::Error error;
    std::string secret = kc::getPassword(package_, service, user_, error);
    if(error)
        throw std::runtime_error(error.message);

    return secret;
}

bool ssave::exists(const std::string& service) {
    kc::Error error;
    std::string secret = kc::getPassword(package_, service, user_, error);
    if(error.type == kc::ErrorType::NotFound)
        return false;
    
    if(error)
        throw std::runtime_error(error.message);

    return true;
}

void ssave::del(const std::string& service) {
    kc::Error error;
    kc::deletePassword(package_, service, user_, error);
    if(error)
        throw std::runtime_error(error.message);
}