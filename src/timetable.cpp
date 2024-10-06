#include "api.hpp"
#include "timetable.hpp"
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
    std::shared_ptr<std::string> url,
    ft::ScreenInteractive* screen_p
) {
    const int TIMETABLE_POSITION = 1;
    
    if(url == nullptr)
        url = std::make_shared<std::string>("");

    // get_timetable will handle empty url
    api::timetable_t timetable_o = api->get_timetable(*url);
    api::events_t events_o = api->get_events();

    prev_url = std::make_shared<std::string>(timetable_o.prev_url);
    next_url = std::make_shared<std::string>(timetable_o.next_url);

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
    const int ACTION_PREV_ENTRY_OFFSET = 1;
    // Add dummy contianer for '<' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    for(size_t i{}; i < weekdays->size() - ACTION_COUNT; i++)
        timetable_container->Add(ft::Container::Vertical({ lessons(timetable_o.timetable[i], events_o) }));

    // Add dummy contianer for '>' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    // On first run
    if(url->empty()) {
        timetable_component->DetachAllChildren();
        
        // If youre seeing this you might ask:
        // "Why the hell did you put those CatchEvents in there, this looks awful!?" well, 
        // this is the only hacky way I found to predictably update the menu selector and have it work
        // Enjoy :)
        ft::Component menu = ft::Menu(*weekdays, selector, ft::MenuOption::HorizontalAnimated())
        | ft::CatchEvent([=](ft::Event event) {
            screen_p->PostEvent(ft::Event::Special("Why"));
            return false;
        })
        | ft::CatchEvent([=](ft::Event e){
            if(e == ft::Event::Special("Why")) {
                if(*selector == 0) {
                    *selector = weekdays->size() - ACTION_COUNT;
                    timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                    timetable_display(timetable_component, api, selector, prev_url, screen_p);
                    return true;
                }
                else if(*selector == weekdays->size() - 1) {
                    *selector = ACTION_PREV_ENTRY_OFFSET;
                    timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                    timetable_display(timetable_component, api, selector, next_url, screen_p);
                }
                return true;
            }
            return false;
        });
        timetable_contents->Add(menu);
    }

    timetable_contents->Add(timetable_container);

    timetable_component->Add(timetable_contents);

    if(url == nullptr || url->empty())
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
    return ft::Renderer(lessons, [=]{ return lessons->Render() | ft::yframe; });
}

ft::Component timetable::empty_lesson_box() {
    const auto seperator_size = ft::size(ft::WIDTH, ft::EQUAL, 20);
    auto menu = ft::MenuEntry({
        .label = "",
        .transform = [=](const ft::EntryState& s) {
            auto empty_entry = [&](ft::Color color){
                return ft::vbox({
                    ft::separatorEmpty(),
                    ft::separator()
                    | ft::color(color)
                    | seperator_size
                    | ft::center,
                    ft::separatorEmpty()
                });
            };
            if(s.focused) 
                return empty_entry(ft::Color::Green);
            if(s.active)
                return empty_entry(ft::Color::Red);
            return empty_entry(ft::Color::White);
        }
    });
    return ft::Renderer(menu, [=]{ return menu->Render() | ft::hcenter; });

}

ft::Component timetable::lesson_box(const api::lesson_t& lesson) {
    const std::string deliminator = " - ";
    const auto subject_size = ft::size(ft::WIDTH, ft::LESS_THAN, 40);

    auto lesson_box = ft::MenuEntry({
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

            entry | ft::xflex;

            if(s.focused)
                return entry | ft::borderStyled(ft::Color::Green);
            if(s.active)
                return entry | ft::borderStyled(ft::Color::Red);
            
            return entry | ft::border;
        }
    });

    return ft::Renderer(lesson_box, [=]{ return lesson_box->Render() | ft::xflex; });
}

ft::Component timetable::event_box(const api::event_t& event) {
    return ft::MenuEntry({
        .label = event.category,
        .transform = [&](const ft::EntryState& s) {
            auto base = [&](const ft::Color& color) {
                return ft::text(s.label)
                | ft::bold
                | ft::color(ft::Color::Green)
                | ft::center
                | ft::borderStyled(color);
            };

            if(s.focused)
                return base(ft::Color::Green);

            if(s.active)
                return base(ft::Color::Red);

            return base(ft::Color::White);
        }
    });
}