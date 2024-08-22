#include "./utils.hpp"
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>

// Cleanup after execution else segfault wiil occur becuase os goes out of scope
void write_func_cleanup(curlpp::Easy& request) {
    // Setting up the write function saves us from segfault when cl::WriteStream goes out of scope
    request.setOpt(curlpp::options::WriteFunction([](char* data, size_t size, size_t nmemb) {
        return size * nmemb;
    }));
}