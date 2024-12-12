#include <ui/timetable.hpp>
#include <ui/custom_ui.hpp>
#include <api/api.hpp>
#include <misc/utils.hpp>
#include <misc/error_handler.hpp>
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
    const std::string& url
) {
    auto loading_screen = std::jthread([&](std::stop_token s_token){
        if(url.empty())
            return;

        int spin_counter{};
        auto loading_screen_component = ft::Renderer([&]{
            return ft::vbox({
                ft::separatorEmpty(),
                ft::spinner(utils::SPINNER_TYPE, spin_counter) | ft::flex | ft::center
            }) | ft::size(ft::HEIGHT, ft::GREATER_THAN, 6);
        });

        timetable_contents->Add(loading_screen_component);

        while(!s_token.stop_requested()) {
            std::this_thread::sleep_for(utils::ANIMATION_WAIT);
            spin_counter++;
        }
        loading_screen_component->Detach();
    });

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

    for(size_t i{}; i < weekdays.size() - ACTION_COUNT; i++) {
        auto lessons_component = ft::Container::Vertical({ lessons(timetable_o.timetable[i], events_o) });
        timetable_container->Add(ft::Renderer(lessons_component, [=]{ 
            auto base = lessons_component->Render()
            | ft::vscroll_indicator
            | ft::yframe;
            if(!lessons_component->Focused())
                return base | ft::size(ft::HEIGHT, ft::LESS_THAN, custom_ui::terminal_height());

            return base;
        }));
    }

    // Add dummy contianer for '>' action
    timetable_container->Add({ft::Renderer([]{ return ft::text(""); })});

    // On first run
    if(url.empty()) {
        // WARNING: If you pass timetable_component directly you will create a std::shared_ptr dependency cycly and with that a memory leak
        std::weak_ptr timetable_component_for_lamda = timetable_component;

        *selector = utils::get_day_of_week(api->get_today()) + ACTION_PREV_ENTRY_OFFSET; // Offset by ACTION_PREV_ENTRY_OFFSET because of action prev('<') element
        ft::Component menu = ft::Menu(weekdays, selector, ft::MenuOption::HorizontalAnimated());
        menu = ft::Renderer(menu, [=, this]{
            if(*selector == 0) {
                *selector = weekdays.size() - ACTION_COUNT;
                timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                std::thread([=, this] {
                    timetable_display(std::shared_ptr(timetable_component_for_lamda), api, selector, prev_url);
                }).detach();

            } else if(*selector == weekdays.size() - 1) {
                *selector = ACTION_PREV_ENTRY_OFFSET;
                timetable_contents->ChildAt(TIMETABLE_POSITION)->Detach();
                std::thread([=, this] {
                    timetable_display(std::shared_ptr(timetable_component_for_lamda), api, selector, next_url);
                }).detach();
            }
            return menu->Render();
        });

        timetable_contents->Add(menu);
    }
    
    loading_screen.request_stop();
    timetable_component->DetachAllChildren();

    timetable_contents->Add(timetable_container);

    timetable_component->Add(timetable_contents);
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
    std::shared_ptr show = std::make_shared<bool>(0);

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
                ft::Elements base = {
                    ft::hbox({
                        ft::text(s.label)
                        | ft::bold,
                        ft::text(" - Substitution")
                    })
                    | ft::color(config_p->Colors().get_accent_color2())
                    | ft::hcenter,
                    ft::text(lesson.start + deliminator + lesson.end)
                    | ft::hcenter
                };

                if(*show)
                    base.emplace(base.begin() + 1, ft::text(lesson.teacher) | ft::hcenter);

                return custom_ui::focus_managed_border_box(
                    ft::vbox(base) | ft::xflex,
                    { .active = s.active, .focused = s.focused }
                );
            } else {
                ft::Elements base = {
                    ft::text(s.label)
                    | ft::bold
                    | ft::color(config_p->Colors().get_main_color())
                    | ft::hcenter,
                    ft::text(lesson.start + deliminator + lesson.end)
                    | ft::hcenter
                };

                if(*show)
                    base.emplace(base.begin() + 1, ft::text(lesson.teacher) | ft::hcenter);

                return custom_ui::focus_managed_border_box(
                    ft::vbox(base) | ft::xflex,
                    { .active = s.active, .focused = s.focused }
                );
            }
        }
    })
    | custom_ui::on_action([=]{ *show = !(*show); });
}

ft::Component timetable::event_box(const api::event_t& event) {
    std::shared_ptr<bool> show = std::make_shared<bool>(0);
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
    })
    | ft::Modal(event_detail_box(event), show.get())
    | custom_ui::on_action([=]{ *show = !(*show); });
}

ft::Component timetable::event_detail_box(const api::event_t& event) {
    const auto max_details_size = ft::size(ft::WIDTH, ft::LESS_THAN, 50);
    return ft::MenuEntry({
        .label = event.category,
        .transform = [=, this](const ft::EntryState& s) {
            return custom_ui::focus_managed_border_box(
                ft::vbox({
                    ft::paragraph(event.category),
                    ft::separator(),
                    ft::paragraph("Description: " + event.description),
                    ft::paragraph("Date: " + event.date),
                    ft::paragraph("Created by: " + event.created_by)
                }),
                { .active = s.active, .focused = s.focused }
            )
            | max_details_size
            | ft::center;
        }
    });
}