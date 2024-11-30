#include <ui/dashboard.hpp>
#include <ui/custom_ui.hpp>
#include <ftxui/component/component.hpp>
#include <memory>

ft::Component dashboard::grades_dashboard::get_component(api* api) {
    const auto grades_o = api->get_grades_unstructured();
    auto grade_component = ft::Container::Vertical({});

    for (const auto& grade : grades_o) {
        grade_component->Add(ft::MenuEntry({
            .label = grade.subject,
            .transform = [=](const ft::EntryState& s) {
                return custom_ui::focus_managed_window(
                    ft::text(s.label), 
                    ft::text(grade.grade + ", " + grade.category), 
                    { .active = s.active, .focused = s.focused }
                );
            }
        }));
    }

    grade_component = ft::Renderer(grade_component, [=, this]{
        return grade_component->Render() 
        | ft::vscroll_indicator
        | ft::yframe
        | ft::reflect(box_);
    }) | switch_focusable_component();

    return custom_ui::custom_component_window(ft::text("Recent grades"), grade_component);
}
