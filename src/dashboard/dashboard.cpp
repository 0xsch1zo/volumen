#include "dashboard.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

ft::Component dashboard::dashboard_display(ft::Component dashboard_component, api* api) {
    return timetable_dashboard::get_timetable_widget(api);
}