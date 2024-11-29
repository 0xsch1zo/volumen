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

    grade_component |= switch_focusable_component();

    return ft::Renderer(grade_component, [this, grade_component, active = active] {
        const std::string grade_window_name = "Recent grades";

        return custom_ui::focus_managed_window(
            ft::text(grade_window_name)
            | ft::hcenter,
            grade_component->Render() | ft::vscroll_indicator | ft::yframe,
            { .active = active, .focused = grade_component->Focused() }
        ) | ft::reflect(box_);
    }) | ft::Hoverable(&active);
}
