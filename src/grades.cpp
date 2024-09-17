#include "grades.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

void grades::grades_display(ft::Component grades_component, api* api) {
    const auto grade_box_size = ft::size(ft::WIDTH, ft::EQUAL, 3);
    std::shared_ptr<api::grades_t> grades_p = api->get_grades();
    ft::Component grades_menu = ft::Container::Vertical({});

    for(const auto& subject : *grades_p) {
        if(subject.second.grades.empty()) {
            grades_menu->Add(empty_subject(subject.second.subject));
            continue;
        }

        auto subject_component = ft::Container::Horizontal({});

        for(const auto& grade : subject.second.grades)
            subject_component->Add( grade_box(grade) );

        grades_menu->Add(ft::Renderer(subject_component, [=]{ 
                return ft::window(
                    ft::text(subject.second.subject), 
                    subject_component->Render()
                ); 
        }));
    }

    grades_component->DetachAllChildren();
    grades_component->Add(ft::Renderer(grades_menu, [&]{ return grades_menu->Render() | ft::yframe; } ));
}

ft::Component grades::grade_box(const api::grade_t& grade) {
    return ft::MenuEntry({
                .label = grade.grade,
                .transform = [=](const ft::EntryState& s) {
                    auto base = [&](ft::Color color){
                        return ft::vbox({
                            ft::text(grade.category),
                            ft::vbox({
                                ft::separator(),
                                ft::text("Grade: " + grade.grade),
                                ft::text("Comment: " + grade.comment),
                                ft::text("Added by: " + grade.added_by),
                                ft::text("Date: " + grade.date),
                            }) | ft::color(ft::Color::White)
                        })
                        | ft::borderStyled(color);    
                        

                    };

                    if(s.focused)
                        return base(ft::Color::Green);

                    if(s.active)
                        return base(ft::Color::Red);

                    return base(ft::Color::White);
                }
            });
}

ft::Component grades::empty_subject(const std::string& subject) {
    return ft::MenuEntry({
        .label = subject,
        .transform = [=](const ft::EntryState& s) {
            const auto empty_placeholder_size = ft::size(ft::WIDTH, ft::EQUAL, 10);
            auto subject_with_placeholder_for_empty = [&](ft::Color color){
                return ft::window(
                    ft::text(s.label)
                    | ft::bold,
                    ft::separator()
                    | ft::color(color)
                    | empty_placeholder_size
                    | ft::hcenter
                );
            };

            if(s.focused)
                return subject_with_placeholder_for_empty(ft::Color::Green);

            if(s.active)
                return subject_with_placeholder_for_empty(ft::Color::Red);

            return subject_with_placeholder_for_empty(ft::Color::White);
        }
    });
}