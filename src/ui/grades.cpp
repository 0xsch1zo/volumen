#include <ui/grades.hpp>
#include <ui/custom_ui.hpp>
#include <misc/utils.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

void grades::grades_display(ft::Component grades_component, api* api) {
    api::grades_t grades_o = api->get_grades();
    ft::Component grades_menu = ft::Container::Vertical({});

    int i{};
    for(const auto& subject : grades_o) {
        if(subject.second.grades.empty()) {
            grades_menu->Add(empty_subject(subject.second.subject));
            continue;
        }

        auto subject_component = ft::Container::Horizontal({});

        for(const auto& grade : subject.second.grades)
            subject_component->Add( grade_box(grade) );

        subject_component = ft::Renderer(subject_component, [=]{ 
            return subject_component->Render()
            | ft::xframe;
        });
        
        grades_menu->Add(custom_ui::custom_component_window(
            ft::text(subject.second.subject), subject_component)
        );

        i++;
    }

    grades_component->DetachAllChildren();
    grades_component->Add(ft::Renderer(grades_menu, [=]{ 
        return grades_menu->Render() 
        | ft::vscroll_indicator
        | ft::yframe
        | ft::size(ft::HEIGHT, ft::LESS_THAN, custom_ui::terminal_height());
    }));
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
                            utils::split("Comment: " + grade.comment.value_or("N/A")),
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