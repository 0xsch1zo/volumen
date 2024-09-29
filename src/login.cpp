#include "login.hpp"
#include "authorization.hpp"
#include "tab.hpp"
#include "utils.hpp"
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

// Function that displays login interface and gets email and password
void login::login_screen(){
    int auth_status{};
    std::string email;
    std::string password;
        
    // Login screen
    ft::Component email_box = utils::custom_component_window(
        ft::text("Email"),
        ft::Input(&email, utils::plain_input())
    );

    ft::InputOption password_opt = utils::plain_input();
    password_opt.password = true;

    ft::Component password_box = utils::custom_component_window(
        ft::text("Password"),
        ft::Input(&password, password_opt)
    );

    ft::Component login_button = ft::Button("Login", [&] { 
        if((auth_status = authorization::authorize(email, password)) == -1) return true;; 
        screen.Exit();
        return true; 
    }, utils::button_rounded());

    ft::Component error_msg = ft::Maybe(ft::Renderer([&] { 
        return ft::text("Login failed! Make sure your login and password are correct. View the backtrace after exiting");
    }), [&]{ if(auth_status == -1) return true; else return false; });

    email_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { password_box->TakeFocus(); return true; }
        return false;
    }); 

    password_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { 
            login_button->TakeFocus();
            if((auth_status = authorization::authorize(email, password)) == -1) return true; 
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
            ft::vbox(utils::split(splash)) | ft::hcenter,
            ft::vbox({
                email_box->Render(),
                password_box->Render(),
                ft::separatorEmpty()
            }) 
            | LOGIN_INTERFACE_SIZE 
            | ft::hcenter,
            login_button->Render() 
            | ft::hcenter,
            ft::separatorEmpty(),
            error_msg->Render() 
            | ft::color(ft::Color::Red)
            | ft::hcenter
        })
        | ft::vcenter;
    });
     
    screen.Loop(login_screen);
    if(auth_status == 0 || auth_status == -1) return;
    choose_account_screen();
}

void login::choose_account_screen() {
    // Choose synergia account
    int synergia_account_i = 0;
    ft::Component info = ft::Renderer([](){ return ft::text("Please choose a synergia account that you want to use"); });

    ft::Component continue_button = ft::Button("Continue", [](){ screen.Exit(); }, utils::button_rounded());
    std::vector<std::string> names;

    for(auto account : authorization::get_synergia_accounts()) {
        names.push_back(account.student_name);
    }

    auto account_menu = utils::custom_dropdown(&names, &synergia_account_i);

    auto choose_synergia_account_components = ft::Container::Vertical({
        info,
        account_menu,
        continue_button
    });

    const auto DMENU_SIZE = ft::size(ft::WIDTH, ft::LESS_THAN, 60);
    const auto BUTTON_SIZE = ft::size(ft::WIDTH, ft::EQUAL, 40);

    auto choose_synergia_account = ft::Renderer(choose_synergia_account_components, [&] {
        return ft::vbox({
            ft::vbox(utils::split(splash)) | ft::hcenter,
            ft::separatorEmpty(),
            ft::vbox({
                info->Render() | ft::color(ft::Color::Green),
                ft::separatorEmpty(),
                account_menu->Render(), 
                continue_button->Render() 
                | BUTTON_SIZE 
                | ft::hcenter
            }) 
            | DMENU_SIZE 
            | ft::hcenter
        })
        | ft::vcenter;
    });

    screen.Loop(choose_synergia_account);
    tab::display_interface(synergia_account_i);
}