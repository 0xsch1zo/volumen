#include "dashboard.hpp"
#include "../custom_ui.hpp"
#include "../utils.hpp"

namespace ft = ftxui;

ft::Component dashboard::weekend_bar::get_weekend_bar(api* api) {
    const int FRIDAY = 5;
    const int SATURDAY = 6;
    const int SUNDAY = 7;
    std::string weekend_text;
    const int SCHOOL_DAY_COUNT = 5;
    int day_of_week = utils::get_day_of_week(api->get_today()) + 1;

    if(day_of_week == SATURDAY || day_of_week == SUNDAY)
        weekend_text = "WEEKEND!!!";
    else
        weekend_text = "Time left to weekend " + std::to_string(SCHOOL_DAY_COUNT + 1 - day_of_week);

    return ft::MenuEntry({ 
        .label = weekend_text,
        .transform = [=](const ft::EntryState& s) {
            return custom_ui::focus_managed_window(
                ft::text(s.label),
                ft::gauge(
                    (day_of_week == FRIDAY) ? 0.98f : static_cast<float>(day_of_week) / (SCHOOL_DAY_COUNT + 1)
                ),
                { .active = s.active, .focused = s.focused }
            );
        }
    });
}