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
            }));
            continue;
        }

        auto grade_box = [&](const std::string& grade){
            return ft::MenuEntry({
                .label = grade,
                .transform = [=](const ft::EntryState& s) {
                    auto base = [&](ft::Color color){
                        return ft::text(s.label)
                        | ft::hcenter
                        | ft::color(ft::Color::White)
                        | grade_box_size
                        | ft::borderStyled(color);    
                    };

                    if(s.focused)
                        return base(ft::Color::Green);

                    if(s.active)
                        return base(ft::Color::Red);

                    return base(ft::Color::White);
                }
            });
        };

        auto subject_component = ft::Container::Horizontal({});

        for(const auto& grade : subject.second.grades)
            subject_component->Add( grade_box(grade.grade) );

        grades_menu->Add(ft::Renderer(subject_component, [=]{ 
                return ft::window(
                    ft::text(subject.second.subject), 
                    subject_component->Render()
                ); 
        }));
    }

    grades_component->DetachAllChildren();
    grades_component->Add(ft::Renderer(grades_menu, [&]{ return grades_menu->Render() | ft::yframe; }));
}