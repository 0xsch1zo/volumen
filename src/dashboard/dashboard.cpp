#include "dashboard.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

ft::Component dashboard::dashboard_display(std::vector<std::string>& timeline, std::vector<std::string>& lessons, int& selected, std::function<void()> on_enter) {

    auto timeline_widget = timetable::get_timeline_widget(timeline);
    auto timetable_widget = timetable::get_timetable_widget(lessons, on_enter);
    auto components = ft::Container::Horizontal({
        timeline_widget,
        timetable_widget
    });


    return std::move(ft::Renderer(components, [=] {
        auto timetable = ft::hbox({
            timeline_widget->Render(),
            ft::separator(),
            timetable_widget->Render(),
        });
        if(timetable_widget->Focused()) 
            timetable = ft::window(ft::text("Timetable") | ft::hcenter, timetable | ft::color(ft::Color::White)) | ft::color(ft::Color::Green);
        else
            timetable = ft::window(ft::text("Timetable") | ft::hcenter, timetable );

        return timetable;
    }));
}