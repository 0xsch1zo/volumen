#include "dashboard.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

ft::Component dashboard::timetable::get_timetable_widget(std::vector<std::string>& lessons, std::function<void()> on_change) {
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

ft::Component dashboard::timetable::get_timeline_widget(std::vector<std::string>& timeline) {
    std::vector<ft::Element> entries;
    for(auto entry : timeline)
        entries.push_back(ft::text(entry));
    
    return ft::Renderer([=]{ return ft::vbox({entries}); });
}