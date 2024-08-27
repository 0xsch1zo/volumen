#include "utils.hpp"
#include <string>
#include <sstream>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <ftxui/dom/elements.hpp>

// Cleanup after execution else segfault wiil occur becuase os goes out of scope
void utils::write_func_cleanup(curlpp::Easy& request) {
    // Setting up the write function saves us from segfault when cl::WriteStream goes out of scope
    request.setOpt(curlpp::options::WriteFunction([](char* data, size_t size, size_t nmemb) {
        return size * nmemb;
    }));
}

// Split text to individual lines. This helps to display ascii art properly among other things
ft::Elements utils::split(std::string text) {
        ft::Elements output;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line, '\n'))
            output.push_back(ft::text(line));
        return output;
}