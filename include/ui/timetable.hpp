#pragma once
#include <api/api.hpp>
#include <misc/config.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ft = ftxui; 

class timetable {
    ft::Component timetable_contents = ft::Container::Vertical({});
    const config* config_p;
    std::string prev_url;
    std::string next_url;

    ft::Component lessons(
        const std::vector<api::lesson_t>& day, 
        api::events_t& events_p
    );
    ft::Component empty_lesson_box();
    ft::Component lesson_box(const api::lesson_t& lesson);
    ft::Component event_box(const api::event_t& event);
    ft::Component event_detail_box(const api::event_t& event);

public:
    timetable(const config* config) : config_p(config) {}
    void timetable_display(
        ft::Component timetable_component, 
        api* api, 
        int* selector,
        const std::string& url,
        ft::ScreenInteractive* screen_p
    );
};