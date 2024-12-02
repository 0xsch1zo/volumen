#include <ui/custom_ui.hpp>
#include <ui/main_ui.hpp>
#include <spdlog/spdlog.h>

using Charset = std::array<std::string, 6>;  // NOLINT
using Charsets = std::array<Charset, 6>;     // NOLINT
// NOLINTNEXTLINE
static Charsets simple_border_charset = {
    Charset{"┌", "┐", "└", "┘", "─", "│"},  // LIGHT
    Charset{"┏", "┓", "┗", "┛", "╍", "╏"},  // DASHED
    Charset{"┏", "┓", "┗", "┛", "━", "┃"},  // HEAVY
    Charset{"╔", "╗", "╚", "╝", "═", "║"},  // DOUBLE
    Charset{"╭", "╮", "╰", "╯", "─", "│"},  // ROUNDED
    Charset{" ", " ", " ", " ", " ", " "}   // EMPTY
};

void custom_ui::init(const config* config) {
    config_p = config;
}

// This provides a button with centered text and rounded corners, yeah ...
ft::ButtonOption custom_ui::button_rounded()
{
    auto option = ft::ButtonOption::Border();
    option.transform = [](const ft::EntryState &s)
    {
        auto element = ft::text(s.label) | ft::center | ft::borderRounded | ft::size(ft::WIDTH, ft::EQUAL, 40);
        if (s.focused)
            return element | ft::bold | ft::color(config_p->Colors().get_main_color());

        if (s.active)
            return element | ft::bold | ft::color(config_p->Colors().get_accent_color1());

        return element;
    };
    return option;
}

ft::Element custom_ui::focus_managed_window(ft::Element title, ft::Element contents, const focus_management_t& focus_management)
{
    auto base = [&](ft::Color title_color, ft::Color overall_color){
        return ft::window(
            title | ft::color(title_color),
            contents | ft::color(ft::Color::White)
        ) | ft::color(overall_color); 
    };
    if (focus_management.focused)
        return base(config_p->Colors().get_accent_color2(), config_p->Colors().get_main_color());

    if (focus_management.active)
        return base(config_p->Colors().get_main_color(), config_p->Colors().get_accent_color1());

    return base(config_p->Colors().get_main_color(), ft::Color::Default);
}

ft::Component custom_ui::custom_component_window(ft::Element title, ft::Component contents) {
    std::shared_ptr box = std::make_shared<ft::Box>();
    std::shared_ptr hovered = std::make_shared<bool>();

    return ft::Renderer(contents, [=] {
        return focus_managed_window(title, contents->Render(), { .active = *hovered, .focused = contents->Focused() }) | ft::reflect(*box);
    })
    // Custom hover implementation because ft::Hoverable doesn't use shared_ptr
    | ft::CatchEvent([=](ft::Event event){
        *hovered = event.is_mouse() && box->Contain(event.mouse().x, event.mouse().y);
        return false;
    });
}

ft::Component custom_ui::content_boxes(const std::vector<api::content_t*>& contents, int* selector) {
    const size_t PREVIEW_SIZE = 600;

    const std::string deliminator = " | ";
    auto content_entries = ft::Container::Vertical({});
    
    for(int i{}; i < contents.size(); i++) {
        const std::string& content = contents.at(i)->content;
        content_entries->Add(ft::MenuEntry({
            .label = (content.size() < PREVIEW_SIZE) ? content : content.substr(0, PREVIEW_SIZE) + "...",
            .transform = [=](const ft::EntryState &s) {
                if(s.focused)
                    *selector = i;
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

    return ft::Renderer(content_entries, [=]{ 
        return content_entries->Render()
        | ft::vscroll_indicator 
        | ft::yframe
        | ft::size(ft::HEIGHT, ft::LESS_THAN, terminal_height() - main_ui::top_menu_size);
    });
}

ft::Element custom_ui::focus_managed_border_box(ft::Element contents, const focus_management_t& focus_management) {
    if(focus_management.focused)
        return contents 
        | ft::borderStyled(config_p->Colors().get_main_color());
    if(focus_management.active)
        return contents 
        | ft::borderStyled(config_p->Colors().get_accent_color1());
    return contents | ft::border; 
}

ft::Element custom_ui::focus_managed_whatever(ft::Element contents, const focus_management_t& focus_management) {
    if(focus_management.focused)
        return contents 
        | ft::color(config_p->Colors().get_main_color());
    if(focus_management.active)
        return contents 
        | ft::color(config_p->Colors().get_accent_color1());
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
                menu |= handle->Focused() ? ft::borderStyled(config_p->Colors().get_main_color()) : ft::border;

                return menu;
            }
            auto menu = ft::vbox({ std::move(checkbox_element), ft::filler() });
            menu |= handle->Focused() ? ft::borderStyled(config_p->Colors().get_main_color()) : ft::border;
            return menu;
        }
    });

    return handle;
}

int custom_ui::terminal_height() {
    return ft::Terminal::Size().dimy;
}