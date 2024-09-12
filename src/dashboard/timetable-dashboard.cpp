#include "dashboard.hpp"
#include "../api.hpp"
#include "../timetable.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

// TODO: implement caching 
ft::Component dashboard::timetable_dashboard::get_timetable_widget(api* api) {

    const std::shared_ptr<api::timetable_t> timetable_p = api->get_timetable();
    int today = timetable::get_day_of_week(api);
    static int selector{};

    auto timetable_widget = ft::Container::Vertical({});
    for(const auto& lesson : *timetable_p->timetable[today]) {
        if(lesson.is_empty)
            continue;
        timetable_widget->Add(ft::MenuEntry(lesson.subject));
    }
    
    auto timeline_widget = timetable_dashboard::get_timeline_widget(timetable_p->timetable[today]);

    return ft::Renderer(timetable_widget, [=] {
        auto timetable = ft::hbox({
            timeline_widget,
            ft::separator(),
            timetable_widget->Render(),
        });
        if(timetable_widget->Focused()) 
            timetable = ft::window(ft::text("Timetable") | ft::hcenter, timetable | ft::color(ft::Color::White)) | ft::color(ft::Color::Green);
        else
            timetable = ft::window(ft::text("Timetable") | ft::hcenter, timetable );

        return timetable;
    });
    
}

ft::Element dashboard::timetable_dashboard::get_timeline_widget(std::shared_ptr<std::vector<api::lesson_t>> day) {
    const std::string deliminator = " - ";
    std::vector<ft::Element> entries;
    ft::Element el;

    for(const auto& lesson: *day) {
        if(lesson.is_empty)
            continue;
        entries.push_back(ft::text(lesson.start + deliminator + lesson.end));
    }
    
    return ft::vbox({entries});
}