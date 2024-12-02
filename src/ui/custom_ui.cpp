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

ft::Element custom_ui::custom_window(ft::Element title, ft::Element contents) {
    using namespace ftxui;
    class custom_border : public Node {
    public:
    custom_border(Elements children,
          BorderStyle style,
          std::optional<Color> foreground_color = std::nullopt)
       : Node(std::move(children)),
         charset_(simple_border_charset[style])  // NOLINT
         ,
         foreground_color_(foreground_color) {}  // NOLINT
  
    const Charset& charset_;  // NOLINT
    std::optional<Color> foreground_color_;
  
    void ComputeRequirement() override {
        Node::ComputeRequirement();
        requirement_ = children_[0]->requirement();
        requirement_.min_x += 2;
        requirement_.min_y += 2;
        if (children_.size() == 2) {
          requirement_.min_x =
              std::max(requirement_.min_x, children_[1]->requirement().min_x + 2);
        }
        requirement_.selected_box.x_min++;
        requirement_.selected_box.x_max++;
        requirement_.selected_box.y_min++;
        requirement_.selected_box.y_max++;
   }
  
   void SetBox(Box box) override {
        Node::SetBox(box);
        if (children_.size() == 2) {
          Box title_box;
          title_box.x_min = box.x_min + 1;
          title_box.x_max = box.x_max - 1;
          title_box.y_min = box.y_min;
          title_box.y_max = box.y_min;
          children_[1]->SetBox(title_box);
        }
        box.x_min++;
        box.x_max--;
        box.y_min++;
        box.y_max--;
        children_[0]->SetBox(box);
   }
  
   void Render(Screen& screen) override {
        // Draw content.
        children_[0]->Render(screen);
    
        // Draw the border.
        if (box_.x_min >= box_.x_max || box_.y_min >= box_.y_max) {
          return;
        }
    
        screen.at(box_.x_min, box_.y_min) = charset_[0];  // NOLINT
        screen.at(box_.x_max, box_.y_min) = charset_[1];  // NOLINT
        screen.at(box_.x_min, box_.y_max) = charset_[2];  // NOLINT
        screen.at(box_.x_max, box_.y_max) = charset_[3];  // NOLINT
    
        for (int x = box_.x_min + 1; x < box_.x_max; ++x) {
          Pixel& p1 = screen.PixelAt(x, box_.y_min);
          Pixel& p2 = screen.PixelAt(x, box_.y_max);
          p1.character = charset_[4];  // NOLINT
          p2.character = charset_[4];  // NOLINT
          p1.automerge = true;
          p2.automerge = true;
        }

        for (int y = box_.y_min + 1; y < box_.y_max; ++y) {
          Pixel& p3 = screen.PixelAt(box_.x_min, y);
          Pixel& p4 = screen.PixelAt(box_.x_max, y);
          p3.character = charset_[5];  // NOLINT
          p4.character = charset_[5];  // NOLINT
          p3.automerge = true;
          p4.automerge = true;
        }

        // Draw the border color.
        if (foreground_color_) {
          for (int x = box_.x_min; x <= box_.x_max; ++x) {
            screen.PixelAt(x, box_.y_min).foreground_color = *foreground_color_;
            screen.PixelAt(x, box_.y_max).foreground_color = *foreground_color_;
          }
          for (int y = box_.y_min; y <= box_.y_max; ++y) {
            screen.PixelAt(box_.x_min, y).foreground_color = *foreground_color_;
            screen.PixelAt(box_.x_max, y).foreground_color = *foreground_color_;
          }
        }

        // Draw title.
        if (children_.size() == 2) {
          children_[1]->Render(screen);
        }
    }
 };
    return std::make_shared<custom_border>(unpack(std::move(contents), std::move(title | ft::notflex)), ROUNDED);
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
        return custom_window(
            ft::hbox({ ft::hbox(title| ft::color(title_color)) | ft::flex}) | ft::flex,
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