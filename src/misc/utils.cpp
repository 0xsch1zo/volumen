#include <misc/utils.hpp>
#include <string>
#include <sstream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

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


std::function<bool(ft::Event)> utils::exit_on_keybind(const std::function<void()>& screen_exit) {
    return [&](ft::Event event){
        if(event == ft::Event::Q || event == ft::Event::q) {
            screen_exit();
            return true;
        }
        return false;
    };
}