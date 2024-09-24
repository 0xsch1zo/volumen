#include "utils.hpp"
#include <string>
#include <sstream>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

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
            output.push_back(ft::paragraph(line));
        return output;
}

int utils::get_day_of_week(std::string&& date_unformated) {
    const std::string delimiter = "-";

    const int DAY_MONTH_YEAR = 3;
    int date[DAY_MONTH_YEAR] = { 0, 0, 0 };

    for(int i{ DAY_MONTH_YEAR - 1 }; i >= 0; i--) {
        int temp_i = date_unformated.find(delimiter);
        date[i] = std::stoi(date_unformated.substr(0, temp_i));
        date_unformated.erase(0, temp_i + delimiter.length());
    }

    std::tm date_in = { 0, 0, 0,
        date[0],
        date[1] - 1,
        date[2] - 1900
    };

    std::time_t date_temp = std::mktime(&date_in);
    const std::tm* date_out = std::localtime(&date_temp);
    // if sunday return 6
    if(date_out->tm_wday == 0)
        return 6;
    else
        return date_out->tm_wday - 1;
}
// This provides a button with centered text and rounded corners, yeah ...
ft::ButtonOption utils::button_rounded() {
    auto option = ft::ButtonOption::Border();
    option.transform = [](const ft::EntryState& s) {
        auto element = ft::text(s.label) | ft::center | ft::borderRounded | ft::size(ft::WIDTH, ft::EQUAL, 40);
        if (s.focused) {
            element |= ft::bold;
            element |= ft::bgcolor(ft::Color::White);
            element |= ft::color(ft::Color::Black);
        }
        return element;
    };
    return option;
}