#include <ui/dashboard.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

void dashboard::dashboard_display(ft::Component dashboard_component, api* api) {
    grades_dashboard grades_dashboard_o;
    timetable_dashboard timetable_dashboard_o;

    auto flex = [](ft::Component component) {
        return ft::Renderer(component, [=]{
            return component->Render()
            | ft::flex;
        });
    };
    auto grades = flex(grades_dashboard_o.get_grades_widget(api));
    auto timetable = flex(timetable_dashboard_o.get_timetable_widget(api));
    auto events = flex(upcoming_events::get_upcoming_events(api));
    auto weekend_bar = flex(weekend_bar::get_weekend_bar(api));
    
    auto dashboard_components = ft::Container::Vertical({
        ft::Container::Horizontal({
            ft::Container::Vertical({
                grades,
                timetable
            }),
            events
        }),
        ft::Container::Horizontal({
            weekend_bar
        })
    });

    // Remove loading screen
    dashboard_component->DetachAllChildren();
    dashboard_component->Add(dashboard_components);
}
