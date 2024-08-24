#include "tab.hpp"
#include "homescreen.hpp"
#include "messages.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

// TODO: remove unnecessary includes
void tab::display_interface() {
    std::vector<std::string> menu = {"Homescreen",
                                    "Messages"};

    int tab_selected{};
    // -------------------------
    // Test variables
    // -------------------------
    int mesd{};
    int sle{};
    std::vector<messages::message_t> msg;
    msg.push_back({
        .subject = "assdasdjdasd",
        .content = "asdasdsdas",
        .author = "asdasd",
    });
    msg.push_back({
        .subject = "assdasdjdasd",
        .content = "asdasdsdas",
        .author = "asdasd",
    });

    // -------------------------
    
    ft::Component tab_menu = ft::Menu(&menu, &tab_selected, ft::MenuOption::HorizontalAnimated());
    ft::Component tab_container = ft::Container::Tab({
        homescreen::homescreen_tab(menu, menu, sle, []{}),
        messages::message_box(msg, mesd, []{})
    }, 
    &tab_selected);
    ft::Component container = ft::Container::Vertical({
        tab_menu,
        tab_container
    });

    auto renderer = ft::Renderer(container, [&]{
        return ft::vbox({
            tab_menu->Render(),
            tab_container->Render()
        });
    });

    auto screen = ft::ScreenInteractive::FullscreenAlternateScreen();
    screen.Loop(renderer);
}