#include "utils.hpp"
#include <string>
#include <sstream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

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

/// @param text should not be double escaped
ft::Elements utils::ansi_color_text_area(const std::string& text) {
    const char ESC = '\e';
    const char escape_terminator = 'm';
    ft::Elements ansi_color_text_area;
    ft::Elements line;
    for(int i{}; i < text.length(); i++) {
        if(text[i] == '\n' || i == text.length() - 1) {
            ansi_color_text_area.push_back(ft::hbox(line));
            line.clear();
        }

        if(text[i] != ESC)
            continue;

        const int terminator_pos = text.find('m', i);
        if(terminator_pos == text.length() - 1)
            break;

        ansi_color color = get_ansi_color(text.substr(i, terminator_pos));
        line.push_back(ft::text(text.substr(terminator_pos + 1, text.find(terminator_pos + 1, ESC))) | ft::color(color.foreground) | ft::bgcolor(color.background));
    }
    if(ansi_color_text_area.size() == 0)
        ansi_color_text_area.push_back(ft::text(text));
    return ansi_color_text_area;
}

utils::ansi_color utils::get_ansi_color(std::string&& escape_sequence) {
    std::runtime_error unsupported_escape_secuence_error("Unsupported color escape sequence\n");
    const char ESC = '\e';
    const char CSI = '[';
    const int EXT_COLOR = 8;
    const int CSI_OFFSET = 1;
    const int FG_BG_OFFSET = 0;
    const int COLOR_OFFSET = 1;
    const std::string COLOR_256_INTERMEDIARY_SEQ = ";5;";
    const std::string COLOR_RGB_INTERMEDIARY_SEQ = ";2;";

    enum color_modes {
        FG = '3',
        BG = '4',
        FG_BRIGHT = '9',
        BG_BRIGHT = '1'
    };

    assert(escape_sequence.starts_with(ESC));
    escape_sequence.erase(escape_sequence.begin());

    if(!escape_sequence.starts_with(CSI))
        throw unsupported_escape_secuence_error;
    
    escape_sequence.erase(escape_sequence.begin());

    utils::ansi_color color;
    while(!escape_sequence.empty()) {
        char color_introducer = escape_sequence.front();
        escape_sequence.erase(escape_sequence.begin());

        bool is_ext_color;
        int color16 = escape_sequence.front() - '0';
        if(color16 == EXT_COLOR) {
            is_ext_color = true;
            color16 = -1;
        }
        escape_sequence.erase(escape_sequence.begin());

        bool is_fg{};
        switch(color_introducer) {
            case FG:
                is_fg = true;
                break;

            case BG:
                is_fg = false;
                break;

            case FG_BRIGHT:
                is_fg = true;
                color16 += 8;
                break;

            case BG_BRIGHT:
                is_fg = false;
                color16 += 8;
                break;

            default:
                throw unsupported_escape_secuence_error;
        }

        if(color16 >= 0 && color16 <= 15) {
            is_fg ? color.foreground = static_cast<ft::Color::Palette16>(color16) : color.background = static_cast<ft::Color::Palette16>(color16);
            continue;

        } else if(is_ext_color && escape_sequence.starts_with(COLOR_256_INTERMEDIARY_SEQ)) {
            escape_sequence.erase(0, (size_t)COLOR_256_INTERMEDIARY_SEQ.length());

            length_and_res id = find_longest_numeric_secuence(escape_sequence);
            escape_sequence.erase(0, (size_t)id.length);
            if(id.res >= 256)
                throw unsupported_escape_secuence_error;

            is_fg ? color.foreground = (ft::Color::Palette256)id.res : color.background = (ft::Color::Palette256)id.res;
            continue;
        } else if(is_ext_color && escape_sequence.starts_with(COLOR_RGB_INTERMEDIARY_SEQ)) {
            escape_sequence.erase(0, (size_t)COLOR_RGB_INTERMEDIARY_SEQ.length());

            length_and_res red = find_longest_numeric_secuence(escape_sequence);
            escape_sequence.erase(0, (size_t)red.length + 1); // Also delete ';'

            length_and_res green = find_longest_numeric_secuence(escape_sequence);
            escape_sequence.erase(0, (size_t)green.length + 1);

            length_and_res blue = find_longest_numeric_secuence(escape_sequence);
            escape_sequence.erase(0, (size_t)blue.length + 1);

            is_fg ? color.foreground = ft::Color(red.res, green.res, blue.res) : color.background = ft::Color(red.res, green.res, blue.res);
        }
        else
            throw unsupported_escape_secuence_error;
    }
    
    return color;
}



utils::length_and_res utils::find_longest_numeric_secuence(const std::string& num) {
    int res{};
    int digit_count{};
    for(size_t i{num.length() - 1}; i >= 0; i++) {
        if(num[i] < '0' || num[i] > '9')
            return { .res = res, .length = digit_count };

        res += 10 * (digit_count++) * num[i];
    }

    return { .res = res, .length = digit_count };
}