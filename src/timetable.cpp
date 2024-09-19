#include "api.hpp"
#include "timetable.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>
#include <memory>


// The format is DD-MM-YYYY
int timetable::get_day_of_week(api* api) {
    std::string date_unformated = api->get_today();
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

void timetable::timetable_display(
    ft::Component timetable_component, 
    api* api, 
    int* selector, 
    std::string url
) {
    
    // get_timetable will handle empty url
    std::shared_ptr<api::timetable_t> timetable_p = api->get_timetable(url);

    auto weekdays = std::make_shared<std::vector<std::string>>();
    *weekdays = {
        "<",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday",
        ">"
    };

    auto timetable_container = ft::Container::Tab({}, selector);

    const int ACTION_COUNT = 2;
    // Add dummy contianer for '<' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    for(size_t i{}; i < weekdays->size() - ACTION_COUNT; i++)
        timetable_container->Add(ft::Container::Vertical({ lessons(timetable_p->timetable[i]) }));

    // Add dummy contianer for '>' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    // On first run
    if(url.empty()) {
        timetable_component->DetachAllChildren();
        ft::Component menu = ft::Menu(*weekdays, selector, ft::MenuOption::HorizontalAnimated())
        | ft::CatchEvent([=](ft::Event event) {
            const int TIMETABLE_POSITION = 1;
            if(*selector == 0) {
                *selector = weekdays->size() - 2;
                timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                timetable_display(timetable_component, api, selector, timetable_p->prev_url);
            }
            else if(*selector == weekdays->size() - 1) {
                *selector = 1;
                timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                timetable_display(timetable_component, api, selector, timetable_p->next_url);
            }
            return false;
        });
        timetable_contents->Add(menu);
    }

    timetable_contents->Add(timetable_container);

    timetable_component->Add(timetable_contents);

    const int ACTION_PREV_ENTRY_OFFSET = 1;
    if(url.empty())
        *selector = get_day_of_week(api) + ACTION_PREV_ENTRY_OFFSET; // Offset by ACTION_PREV_ENTRY_OFFSET because of action prev('<') element
}

ft::Component timetable::lessons(std::shared_ptr<std::vector<api::lesson_t>> day) {
    const std::string deliminator = " - ";
    const auto seperator_size = ft::size(ft::WIDTH, ft::EQUAL, 20);
    const auto subject_size = ft::size(ft::WIDTH, ft::LESS_THAN, 40);
    ft::Component lessons = ft::Container::Vertical({});

    for(auto& lesson : *day) {
        if(lesson.is_empty) {
            lessons->Add(ft::MenuEntry({
            .label = lesson.subject,
            .transform = [=](const ft::EntryState& s) {
                ft::Element empty_entry;
                if(s.focused) {
                    empty_entry = ft::vbox({
                        ft::separatorEmpty(),
                        ft::separator()
                        | ft::color(ft::Color::Green)
                        | seperator_size
                        | ft::center,
                        ft::separatorEmpty()
                    });
                } 
                else if(s.active) {
                    empty_entry = ft::vbox({
                        ft::separatorEmpty(),
                        ft::separator()
                        | ft::color(ft::Color::Red)
                        | seperator_size
                        | ft::center,
                        ft::separatorEmpty()
                    });
                } 
                else {
                    empty_entry = ft::vbox({
                        ft::separatorEmpty(),
                        ft::separator()
                        | seperator_size
                        | ft::center,
                        ft::separatorEmpty()
                    });
                }
                
                return empty_entry;
            }
            }));
            continue;
        }

        lessons->Add(ft::MenuEntry({
            .label = lesson.subject,
            .transform = [=](const ft::EntryState& s) {
                ft::Element entry;

                if(lesson.is_canceled) {
                    entry = ft::vbox({
                        ft::text(s.label)
                        | ft::bold
                        | ft::color(ft::Color::Red)
                        | ft::strikethrough
                        | subject_size
                        | ft::hcenter,
                        ft::text(lesson.start + deliminator + lesson.end)
                        | ft::hcenter
                    });
                }
                else {
                    entry = ft::vbox({
                        ft::text(s.label)
                        | ft::bold
                        | ft::color(ft::Color::Green)
                        | ft::hcenter,
                        ft::text(lesson.start + deliminator + lesson.end)
                        | ft::hcenter
                    });
                }

                if(s.focused)
                    entry = entry | ft::borderStyled(ft::Color::Green);
                else if(s.active)
                    entry = entry | ft::borderStyled(ft::Color::Red);
                else
                    entry = entry | ft::border;
                return entry;
            }
        }));
    }
    return ft::Renderer(lessons, [=]{ return lessons->Render() | ft::yframe; });
}