#include "grades.hpp"
#include "custom_ui.hpp"
#include "utils.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

void grades::grades_display(ft::Component grades_component, api* api) {
    api::grades_t grades_o = api->get_grades();
    ft::Component grades_menu = ft::Container::Vertical({});
    active.resize(grades_o.size());

    int i{};
    for(const auto& subject : grades_o) {
        if(subject.second.grades.empty()) {
            grades_menu->Add(empty_subject(subject.second.subject));
            continue;
        }

        auto subject_component = ft::Container::Horizontal({});

        for(const auto& grade : subject.second.grades)
            subject_component->Add( grade_box(grade) );

        grades_menu->Add(ft::Renderer(subject_component, [=, this]{ 
            return custom_ui::focus_managed_window(
                ft::text(subject.second.subject), 
                subject_component->Render() | ft::xframe,
                { .active = static_cast<bool>(active[i]), .focused = subject_component->Focused() }
            );
        }) | ft::Hoverable((bool*)&active[i])
        );
        i++;
    }

    grades_component->DetachAllChildren();
    grades_component->Add(ft::Renderer(grades_menu, [=]{ return grades_menu->Render() | ft::yframe; } ));
}

ft::Component grades::grade_box(const api::grade_t& grade) {
    return ft::MenuEntry({
        .label = grade.grade,
        .transform = [=](const ft::EntryState& s) {
            const auto max_grade_box_size = ft::size(ft::WIDTH, ft::LESS_THAN, 30);
            return custom_ui::focus_managed_border_box(
                ft::vbox({
                    ft::text(grade.category),
                    ft::vbox({
                        ft::separator(),
                        ft::text("Grade: " + grade.grade),
                        ft::vbox({
                            utils::split("Comment: " + grade.comment),
                        }),
                        ft::text("Added by: " + grade.added_by),
                        ft::text("Date: " + grade.date),
                    }) 
                    | ft::color(ft::Color::White)
                })
                | max_grade_box_size,
                { .active = s.active, .focused = s.focused }
            );
        }
    });
}

ft::Component grades::empty_subject(const std::string& subject) {
    return ft::MenuEntry({
        .label = subject,
        .transform = [=](const ft::EntryState& s) {
            const auto empty_placeholder_size = ft::size(ft::WIDTH, ft::EQUAL, 10);
            return custom_ui::focus_managed_window(
                ft::text(s.label)
                | ft::bold,
                ft::separator()
                | empty_placeholder_size
                | ft::hcenter,
                { .active = s.active, .focused = s.focused }
            );
        }
    });
}