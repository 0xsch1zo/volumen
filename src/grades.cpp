#include "grades.hpp"
#include <ftxui/dom/elements.hpp>

void grades::grades_display(ft::Component grades_component, api* api) {
    const auto grade_box_size = ft::size(ft::WIDTH, ft::EQUAL, 3);
    const auto empty_placeholder_size = ft::size(ft::WIDTH, ft::EQUAL, 10);
    std::shared_ptr<api::grades_t> grades_p = api->get_grades();

    ft::Component grades_menu = ft::Container::Vertical({});
    for(const auto& subject : *grades_p) {
        if(subject.second.grades.empty()) {
            grades_menu->Add(ft::MenuEntry({
            .label = subject.second.subject,
            .transform = [=](const ft::EntryState& s) {
                ft::Element subject_with_placeholder_for_empty = ft::window(
                    ft::text(s.label)
                    | ft::bold,
                    ft::separator()
                    | ft::color(ft::Color::White)
                    | empty_placeholder_size 
                    | ft::hcenter);

                if(s.focused)
                    return subject_with_placeholder_for_empty
                    | ft::color(ft::Color::Green);

                if(s.active)
                    return subject_with_placeholder_for_empty
                    | ft::color(ft::Color::Red);

                return subject_with_placeholder_for_empty;
            }
            }));
            continue;
        }
        grades_menu->Add(ft::MenuEntry({
            .label = subject.second.subject,
            .transform = [=](const ft::EntryState& s) {
                std::vector<ft::Element> grade_boxes;

                for(const auto& grade : subject.second.grades) {
                    grade_boxes.push_back(
                        ft::text(grade.grade)
                        | ft::hcenter
                        | grade_box_size
                        | ft::border
                    );
                }

                ft::Element subject_element = ft::window(
                    ft::text(s.label)
                    | ft::bold,
                    ft::hbox(grade_boxes) | ft::color(ft::Color::White)
                );

                if(s.focused)
                    return subject_element | ft::color(ft::Color::Green);

                if(s.active)
                    return subject_element | ft::color(ft::Color::Red);

                return subject_element;
            }
        }));
    }

    grades_component->DetachAllChildren();
    grades_component->Add(ft::Renderer(grades_menu, [&]{ return grades_menu->Render() | ft::yframe; }));
}