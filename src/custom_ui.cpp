#include "custom_ui.hpp"

// This provides a button with centered text and rounded corners, yeah ...
ft::ButtonOption custom_ui::button_rounded()
{
    auto option = ft::ButtonOption::Border();
    option.transform = [](const ft::EntryState &s)
    {
        auto element = ft::text(s.label) | ft::center | ft::borderRounded | ft::size(ft::WIDTH, ft::EQUAL, 40);
        if (s.focused)
            return element | ft::bold | ft::color(ft::Color::Green);

        if (s.active)
            return element | ft::bold | ft::color(ft::Color::Red);

        return element;
    };
    return option;
}

ft::Element custom_ui::focus_managed_window(ft::Element title, ft::Element contents, const focus_management_t& focus_management)
{
    auto base = ft::window(title, contents | ft::color(ft::Color::White));
    if (focus_management.focused)
        return base | ft::color(ft::Color::Green);

    if (focus_management.active)
        return base | ft::color(ft::Color::Red);

    return base;
}

ft::Component custom_ui::custom_component_window(ft::Element title, ft::Component contents)
{
    return ft::Renderer(contents, [=]
                        {
        if(contents->Focused())
            return ft::window(title, contents->Render() | ft::color(ft::Color::White)) 
            | ft::color(ft::Color::Green);

        return ft::window(title, contents->Render()); });
}

ft::Component custom_ui::content_box(const std::vector<api::content_t*>& contents) {
    const size_t PREVIEW_SIZE = 300;

    const std::string deliminator = " | ";
    auto content_entries = ft::Container::Vertical({});
    
    for(int i{}; i < contents.size(); i++) {
        const std::string& content = contents.at(i)->content;
        content_entries->Add(ft::MenuEntry({
            .label = (content.size() < PREVIEW_SIZE) ? content : content.substr(0, PREVIEW_SIZE) + "...",
            .transform = [=](const ft::EntryState &s) {
                return focus_managed_window(
                    ft::hbox({
                        ft::text(contents.at(i)->subject) | ft::bold,
                        ft::text(deliminator + contents.at(i)->author)
                    }),
                    ft::paragraph(s.label),
                    { .active = s.active, .focused = s.focused }
                );
            }
        }));
    }

    return content_entries;
}

ft::Element custom_ui::focus_managed_border_box(ft::Element contents, const focus_management_t& focus_management) {
    if(focus_management.focused)
        return contents 
        | ft::borderStyled(ft::Color::Green);
    if(focus_management.active)
        return contents 
        | ft::borderStyled(ft::Color::Red);
    return contents | ft::border; 
}

ft::Element custom_ui::focus_managed_whatever(ft::Element contents, const focus_management_t& focus_management) {
    if(focus_management.focused)
        return contents 
        | ft::color(ft::Color::Green);
    if(focus_management.active)
        return contents 
        | ft::color(ft::Color::Red);
    return contents;
}

ft::InputOption custom_ui::plain_input() {
    return {
        .transform = [](ft::InputState s) {
            return s.element;
        }
    };
}

ft::Component custom_ui::custom_dropdown(ft::ConstStringListRef entries, int* selected) {
    ft::Component handle = ft::Dropdown({
        .checkbox = {
            .transform = [](const ft::EntryState& s) {
                auto prefix = ft::text(s.state ? "↓ " : "→ ");  // NOLINT
                auto t = ft::text(s.label);
                if ((s.focused || s.active) && !s.state)
                    t |= ft::bold;
                return ft::hbox({prefix, t});
            }
        },
        .radiobox = {
            .entries = entries,
            .selected = selected,
            .transform = [](const ft::EntryState& s) {
#if defined(FTXUI_MICROSOFT_TERMINAL_FALLBACK)
                // Microsoft terminal do not use fonts able to render properly the default
                // radiobox glyph.
                auto prefix = ft::text(s.state ? "(*) " : "( ) ");  // NOLINT
#else
                auto prefix = ft::text(s.state ? "◉ " : "○ ");  // NOLINT
#endif
                auto e = ft::text(s.label);
                if (s.active || s.focused)
                    e |= ft::bold;

                return ft::hbox({prefix, e});
            }
        },
        .transform = [&](
            bool open, 
            ft::Element checkbox_element, 
            ft::Element radiobox_element
        ) {
            if (open) {
                const int max_height = 12;
                auto menu = ft::vbox({
                    std::move(checkbox_element),
                    ft::separator(),
                    std::move(radiobox_element) 
                    | ft::vscroll_indicator 
                    | ft::frame 
                    | ft::size(ft::HEIGHT, ft::LESS_THAN, max_height),
                });
                menu |= handle->Focused() ? ft::borderStyled(ft::Color::Green) : ft::border;

                return menu;
            }
            auto menu = ft::vbox({ std::move(checkbox_element), ft::filler() });
            menu |= handle->Focused() ? ft::borderStyled(ft::Color::Green) : ft::border;
            return menu;
        }
    });

    return handle;
}