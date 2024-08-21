#include "login.hpp"
#include "authorization.hpp"
#include <sstream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

const std::string login::splash = R"(
 ██▒   █▓ ▒█████   ██▓     █    ██  ███▄ ▄███▓▓█████  ███▄    █ 
▓██░   █▒▒██▒  ██▒▓██▒     ██  ▓██▒▓██▒▀█▀ ██▒▓█   ▀  ██ ▀█   █ 
 ▓██  █▒░▒██░  ██▒▒██░    ▓██  ▒██░▓██    ▓██░▒███   ▓██  ▀█ ██▒
  ▒██ █░░▒██   ██░▒██░    ▓▓█  ░██░▒██    ▒██ ▒▓█  ▄ ▓██▒  ▐▌██▒
   ▒▀█░  ░ ████▓▒░░██████▒▒▒█████▓ ▒██▒   ░██▒░▒████▒▒██░   ▓██░
   ░ ▐░  ░ ▒░▒░▒░ ░ ▒░▓  ░░▒▓▒ ▒ ▒ ░ ▒░   ░  ░░░ ▒░ ░░ ▒░   ▒ ▒ 
   ░ ░░    ░ ▒ ▒░ ░ ░ ▒  ░░░▒░ ░ ░ ░  ░      ░ ░ ░  ░░ ░░   ░ ▒░
     ░░  ░ ░ ░ ▒    ░ ░    ░░░ ░ ░ ░      ░      ░      ░   ░ ░ 
      ░      ░ ░      ░  ░   ░            ░      ░  ░         ░ 
     ░                                                          
)";
ft::ScreenInteractive login::screen = ft::ScreenInteractive::FullscreenAlternateScreen();

// Split text to individual lines. This helps to display ascii art properly
ft::Elements login::split(std::string text) {
        ft::Elements output;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line, '\n'))
            output.push_back(ft::text(line));
        return output;
}

// This provides a button with centered text and rounded corners, yeah ...
ft::ButtonOption login::button_style() {
    auto option = ft::ButtonOption::Border();
    option.transform = [](const ft::EntryState& s) {
        auto element = ft::text(s.label) | ft::center | ft::borderRounded | ft::size(ft::WIDTH, ft::EQUAL, 40);
        if (s.focused) {
            element |= ft::bold;
            element |= ft::bgcolor(ft::Color::White);
            element |= ft::color(ft::Color::Black);
        }
        return element;
    };
    return option;
}

// Function that displays login interface and gets email and password
void login::login_screen(){
    bool auth_failure{};
    std::string email;
    std::string password;
        
    // Login screen
    ft::Component email_box = ft::Input(&email);
    ft::InputOption password_opt;
    password_opt.password = true;
    ft::Component password_box = ft::Input(&password, password_opt);
    ft::Component login_button = ft::Button("Login", [&] { 
        auth_failure = !authorization::authorize(email, password); 
        screen.Exit();
        return true; 
    }, button_style());

    ft::Component error_msg = ft::Maybe(ft::Renderer([&] { 
        return ft::text("Login failed! Make sure your login and password are correct. View the backtrace after exiting");
    }), &auth_failure);

    email_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { password_box->TakeFocus(); return true; }
        return false;
    }); 

    password_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { 
            auth_failure = !authorization::authorize(email, password); 
            screen.Exit();
            return true;
        }
        return false;
    });

    auto login_components = ft::Container::Vertical({
        email_box,
        password_box,
        login_button,
        error_msg
    });

    const auto LOGIN_INTERFACE_SIZE = ft::size(ft::WIDTH, ft::EQUAL, 40);
    auto login_screen = ft::Renderer(login_components, [&] {
        return ft::vbox({
                // Could make splash a component but there will be focus issues caused by that if it's a static global
                ft::vbox(split(splash)) | ft::hcenter,
                ft::vbox({
                    ft::window(ft::text("Email"), email_box->Render()),
                    ft::window(ft::text("Password"), password_box->Render()),
                        ft::separatorEmpty()
                    })  | LOGIN_INTERFACE_SIZE | ft::hcenter,
                    login_button->Render() | ft::hcenter,
                    ft::separatorEmpty(),
                    error_msg->Render() | ft::color(ft::Color::Red) | ft::hcenter
                })  |
                ft::vcenter;
    });
     
    screen.Loop(login_screen);
    if(auth_failure) return;
    choose_account_screen();
}

void login::choose_account_screen() {
    // Choose synergia account
    int account_i = 0;
    ft::Component info = ft::Renderer([](){ return ft::text("Please choose a synergia account that you want to use"); });

    ft::Component continue_button = ft::Button("Continue", [&account_i](){ screen.Exit(); }, button_style());
    std::vector<std::string> names;

    for(auto account : authorization::get_synergia_accounts()) {
        names.push_back(account.student_name);
    }

    auto account_menu = ft::Dropdown({
        .radiobox =
        {
            .entries = &names,
            .selected = &account_i,
            .transform =
            [](const ft::EntryState& s) {
                auto t = ft::text(s.label) | ft::borderEmpty;
                if (s.active) {
                    t |= ft::bold;
                }
                if (s.focused) {
                    t |= ft::inverted;
                }
                return t;
            },
        },
    });

    auto choose_synergia_account_components = ft::Container::Vertical({
        info,
        account_menu,
        continue_button
    });

    const auto DMENU_SIZE = ft::size(ft::WIDTH, ft::LESS_THAN, 60);
    const auto BUTTON_SIZE = ft::size(ft::WIDTH, ft::EQUAL, 40);

    auto choose_synergia_account = ft::Renderer(choose_synergia_account_components, [&] {
        return ft::vbox({
            ft::vbox(split(splash)) | ft::hcenter,
            ft::separatorEmpty(),
            ft::vbox({
                info->Render() | ft::color(ft::Color::Green),
                ft::separatorEmpty(),
                account_menu->Render(), 
                continue_button->Render() | BUTTON_SIZE | ft::hcenter
            }) | DMENU_SIZE | ft::hcenter
        })  |
        ft::vcenter;
    });

    screen.Loop(choose_synergia_account);
}