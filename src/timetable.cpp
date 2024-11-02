#include "api.hpp"
#include "timetable.hpp"
#include "custom_ui.hpp"
#include "utils.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>
#include <memory>


void timetable::timetable_display(
    ft::Component timetable_component, 
    api* api, 
    int* selector,
    const std::string& url,
    ft::ScreenInteractive* screen_p
) {
    const int TIMETABLE_POSITION = 1;

    // get_timetable will handle empty url
    api::timetable_t timetable_o = api->get_timetable(url);
    api::events_t events_o = api->get_events();

    prev_url = timetable_o.prev_url;
    next_url = timetable_o.next_url;

    const std::vector<std::string> weekdays = {
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
    const int ACTION_PREV_ENTRY_OFFSET = 1;
    // Add dummy contianer for '<' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    for(size_t i{}; i < weekdays.size() - ACTION_COUNT; i++)
        timetable_container->Add(ft::Container::Vertical({ lessons(timetable_o.timetable[i], events_o) }));

    // Add dummy contianer for '>' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    // On first run
    if(url.empty()) {
        timetable_component->DetachAllChildren();
        // WARNING: If you pass timetable_component directly you will create a std::shared_ptr dependency cycly and with that a memory leak
        std::weak_ptr timetable_component_for_lamda = timetable_component;

        // If youre seeing this you might ask:
        // "Why the hell did you put those CatchEvents in there, this looks awful!?" well, 
        // this is the only hacky way I found to predictably update the menu selector and have it work
        // Enjoy :)
        ft::Component menu = ft::Menu(weekdays, selector, ft::MenuOption::HorizontalAnimated())
        | ft::CatchEvent([screen_p = screen_p](ft::Event event) {
            screen_p->PostEvent(ft::Event::Special("Why"));
            return false;
        })
        | ft::CatchEvent([=, this](ft::Event e){
            if(e == ft::Event::Special("Why")) {
                if(*selector == 0) {
                    *selector = weekdays.size() - ACTION_COUNT;
                    timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                    timetable_display(std::shared_ptr(timetable_component_for_lamda), api, selector, prev_url, screen_p);
                }
                else if(*selector == weekdays.size() - 1) {
                    *selector = ACTION_PREV_ENTRY_OFFSET;
                    timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                    timetable_display(std::shared_ptr(timetable_component_for_lamda), api, selector, next_url, screen_p);
                }
                return true;
            }
            return false;
        });
        timetable_contents->Add(menu);
    }

    timetable_contents->Add(timetable_container);

    timetable_component->Add(timetable_contents);

    if(url.empty())
        *selector = utils::get_day_of_week(api->get_today()) + ACTION_PREV_ENTRY_OFFSET; // Offset by ACTION_PREV_ENTRY_OFFSET because of action prev('<') element
}

ft::Component timetable::lessons(const std::vector<api::lesson_t>& day, api::events_t& events_o) {
    ft::Component lessons = ft::Container::Vertical({});

    int i{};
    for(auto& lesson : day) {
        auto events_component = ft::Container::Horizontal({});
        if(events_o.contains(lesson.date)) {
            // Find event relevant to lesson number
            for(const auto& event : events_o[lesson.date]) {
                if(event.lesson_offset == (i++) + 1)
                    events_component->Add(event_box(event));
            }
        }

        if(lesson.is_empty) {
            if(events_component->ChildCount() >= 1)
                lessons->Add(ft::Container::Horizontal({empty_lesson_box(), events_component}));
            else
                lessons->Add(empty_lesson_box()); 
            continue;
        }

        auto lesson_component = lesson_box(lesson);
        
        if(events_component->ChildCount() >= 1)
            lessons->Add(ft::Container::Horizontal({lesson_component, events_component}));
        else
            lessons->Add(lesson_component); 
    }
    return ft::Renderer(lessons, [=, this]{ return lessons->Render() | ft::yframe; });
}

ft::Component timetable::empty_lesson_box() {
    const auto seperator_size = ft::size(ft::WIDTH, ft::EQUAL, 20);
    return ft::MenuEntry({
        .label = "",
        .transform = [=](const ft::EntryState& s) {
            return custom_ui::focus_managed_whatever(
                ft::vbox({
                    ft::separatorEmpty(),
                    ft::separator()
                    | seperator_size
                    | ft::center,
                    ft::separatorEmpty()
                }),
                { .active = s.active, .focused = s.focused }
            );
        }
    });
}

// TODO: incorporate empty_lesson_box, divide lesson types into separate funcs
ft::Component timetable::lesson_box(const api::lesson_t& lesson) {
    const std::string deliminator = " - ";
    const auto subject_size = ft::size(ft::WIDTH, ft::LESS_THAN, 40);

    return ft::MenuEntry({
        .label = lesson.subject,
        .transform = [=, this](const ft::EntryState& s) {
            if(lesson.is_canceled) {
                return custom_ui::focus_managed_border_box(
                    ft::vbox({
                        ft::text(s.label)
                        | ft::bold
                        | ft::color(config_p->Colors().get_accent_color1())
                        | ft::strikethrough
                        | subject_size
                        | ft::hcenter,
                        ft::text(lesson.start + deliminator + lesson.end)
                        | ft::hcenter
                    }) | ft::xflex,
                    { .active = s.active, .focused = s.focused }
                );
            } else if(lesson.is_substitution) {
                return custom_ui::focus_managed_border_box(
                    ft::vbox({
                        ft::hbox({
                            ft::text(s.label)
                            | ft::bold,
                            ft::text(" - Substitution")
                        })
                        | ft::color(config_p->Colors().get_accent_color2())
                        | ft::hcenter,
                        ft::text(lesson.start + deliminator + lesson.end)
                        | ft::hcenter
                    }) | ft::xflex,
                    { .active = s.active, .focused = s.focused }
                );
            } else
                return custom_ui::focus_managed_border_box(
                    ft::vbox({
                        ft::text(s.label)
                        | ft::bold
                        | ft::color(config_p->Colors().get_main_color())
                        | ft::hcenter,
                        ft::text(lesson.start + deliminator + lesson.end)
                        | ft::hcenter
                    }) | ft::xflex,
                    { .active = s.active, .focused = s.focused }
                );
        }
    });
}

ft::Component timetable::event_box(const api::event_t& event) {
    return ft::MenuEntry({
        .label = event.category,
        .transform = [&](const ft::EntryState& s) {
            return custom_ui::focus_managed_border_box(
                ft::text(s.label)
                | ft::bold
                | ft::color(config_p->Colors().get_main_color())
                | ft::center,
                { .active = s.active, .focused = s.focused }
            );
        }
    });
}