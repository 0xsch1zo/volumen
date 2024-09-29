#pragma once
#include "api.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ft = ftxui; 

class timetable {
    ft::Component timetable_contents = ft::Container::Vertical({});
    std::shared_ptr<std::string> prev_url;
    std::shared_ptr<std::string> next_url;

    static ft::Component lessons(
        std::shared_ptr<std::vector<api::lesson_t>> day, 
        std::shared_ptr<api::events_t> events_p
    );
    static ft::Component empty_lesson_box();
    static ft::Component lesson_box(const api::lesson_t& lesson);
    static ft::Component event_box(const api::event_t& event);

public:
    void timetable_display(
        ft::Component timetable_component, 
        api* api, 
        int* selector,
        std::shared_ptr<std::string> url,
        ft::ScreenInteractive* screen_p
    );
};