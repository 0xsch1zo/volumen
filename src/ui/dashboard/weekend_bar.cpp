#include <ui/dashboard.hpp>
#include <ui/custom_ui.hpp>
#include <misc/utils.hpp>

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

    auto weekend_bar_component = ft::MenuEntry({ 
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

    return ft::Renderer(weekend_bar_component, [=]{
        return weekend_bar_component->Render()
        // Stop the bar from collapsing
        | ft::size(ft::HEIGHT, ft::EQUAL, 4);
    });
}