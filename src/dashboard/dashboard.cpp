#include "dashboard.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

ft::Component dashboard::get_timetable_widget(std::vector<std::string>& lessons, std::function<void()> on_change) {
    int selected{};
    return ft::Radiobox({
        .entries = lessons,
        .selected = &selected,
        .transform = [](const ft::EntryState &s){
            auto element = ft::text(s.label);
            if(s.focused) {
                element |= ft::bold;
                element |= ft::inverted;
            }
            return element;
        },
        .on_change = on_change
    });
}

ft::Component dashboard::get_timeline_widget(std::vector<std::string>& timeline) {
    std::vector<ft::Element> entries;
    for(auto entry : timeline)
        entries.push_back(ft::text(entry));
    
    return ft::Renderer([=]{ return ft::vbox({entries}); });
}

ft::Component dashboard::dashboard_tab(std::vector<std::string>& timeline, std::vector<std::string>& lessons, int& selected, std::function<void()> on_enter) {

    auto timeline_widget = get_timeline_widget(timeline);
    auto timetable_widget = get_timetable_widget(lessons, on_enter);
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