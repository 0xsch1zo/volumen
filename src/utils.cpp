#include "utils.hpp"
#include <string>
#include <sstream>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
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

// This provides a button with centered text and rounded corners, yeah ...
ft::ButtonOption utils::button_rounded() {
    auto option = ft::ButtonOption::Border();
    option.transform = [](const ft::EntryState& s) {
        auto element = ft::text(s.label) | ft::center | ft::borderRounded | ft::size(ft::WIDTH, ft::EQUAL, 40);
        if (s.focused)
            return element 
            | ft::bold
            | ft::color(ft::Color::Green);

        if (s.active)
            return element 
            | ft::bold
            | ft::color(ft::Color::Red);

        return element;
    };
    return option;
}

ft::Component utils::custom_component_window(ft::Element title, ft::Component contents) {
    return ft::Renderer(contents, [=]{
        if(contents->Focused())
            return ft::window(title, contents->Render() | ft::color(ft::Color::White)) 
            | ft::color(ft::Color::Green);

        return ft::window(title, contents->Render());
    });
}

ft::InputOption utils::plain_input() {
    return {
        .transform = [](ft::InputState s){
            return s.element;
        }
    };
}

ft::Component utils::custom_dropdown(ft::ConstStringListRef entries, int* selected) {
    ft::Component handle = ft::Dropdown({
        .checkbox = {
            .transform = [](const ft::EntryState& s) {
                auto prefix = ft::text(s.state ? "↓ " : "→ ");  // NOLINT
                auto t = ft::text(s.label);
                if ((s.focused || s.active) && !s.state)
                    t |= ft::bold;
                return ft::hbox({prefix, t});
            }
        },
        .radiobox = {
            .entries = entries,
            .selected = selected,
            .transform = [](const ft::EntryState& s) {
#if defined(FTXUI_MICROSOFT_TERMINAL_FALLBACK)
                // Microsoft terminal do not use fonts able to render properly the default
                // radiobox glyph.
                auto prefix = ft::text(s.state ? "(*) " : "( ) ");  // NOLINT
#else
                auto prefix = ft::text(s.state ? "◉ " : "○ ");  // NOLINT
#endif
                auto e = ft::text(s.label);
                if (s.active || s.focused)
                    e |= ft::bold;

                return ft::hbox({prefix, e});
            }
        },
        .transform = [&](
            bool open, 
            ft::Element checkbox_element, 
            ft::Element radiobox_element
        ) {
            if (open) {
                const int max_height = 12;
                auto menu = ft::vbox({
                    std::move(checkbox_element),
                    ft::separator(),
                    std::move(radiobox_element) 
                    | ft::vscroll_indicator 
                    | ft::frame 
                    | ft::size(ft::HEIGHT, ft::LESS_THAN, max_height),
                });
                menu |= handle->Focused() ? ft::borderStyled(ft::Color::Green) : ft::border;

                return menu;
            }
            auto menu = ft::vbox({ std::move(checkbox_element), ft::filler() });
            menu |= handle->Focused() ? ft::borderStyled(ft::Color::Green) : ft::border;
            return menu;
        }
    });

    return handle;
}