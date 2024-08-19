#include "login.hpp"
#include "authorization.hpp"
#include <sstream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>


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
void login::ui(){
        auto screen = ft::ScreenInteractive::FullscreenAlternateScreen();
        bool login_failed{};
        std::string email;
        std::string password;
        std::string splash = R"(
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

        ft::Component email_box = ft::Input(&email);
        ft::InputOption password_opt;
        password_opt.password = true;
        ft::Component password_box = ft::Input(&password, password_opt);
        ft::Component login_button = ft::Button("Login", [&] { authorization::authorize(email, password, login_failed); return true; }, button_style());
        ft::Component error_msg = ft::Maybe(ft::Renderer([&] { 
            return ft::text("Login failed! Make sure your login and password are correct. View the backtrace after exiting");
        }), &login_failed);

        email_box |= ft::CatchEvent([&](ft::Event event) {
            if(event == ft::Event::Return) { password_box->TakeFocus(); return true; }
            return false;
        }); 

        password_box |= ft::CatchEvent([&](ft::Event event) {
            if(event == ft::Event::Return) { authorization::authorize(email, password, login_failed); return true; }
            return false;
        });

        auto component = ft::Container::Vertical({
            email_box,
            password_box,
            login_button,
            error_msg
        });

        auto renderer = ft::Renderer(component, [&] {
            return ft::vbox({
                    ft::vbox(split(splash)) | ft::hcenter,
                    ft::vbox({
                        ft::window(ft::text("Email"), email_box->Render()),
                        ft::window(ft::text("Password"), password_box->Render()),
                        ft::separatorEmpty()
                    })  | ft::size(ft::WIDTH, ft::EQUAL, 40) | ft::hcenter,
                    login_button->Render() | ft::hcenter,
                    ft::separatorEmpty(),
                    error_msg->Render() | ft::color(ft::Color::Red) | ft::hcenter
                })  |
                ft::vcenter;
        });

        screen.Loop(renderer);
}