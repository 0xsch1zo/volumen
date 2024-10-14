#include "login.hpp"
#include "tab.hpp"
#include "utils.hpp"
#include "custom_ui.hpp"
#include <sstream>
#include <spdlog/spdlog.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

// Function that displays login interface and gets email and password
void login::login_screen(){
    bool auth_failure{};
    bool auth_complete{};
    auth auth;
    std::string email;
    std::string password;

    auto authorize = [&] { 
        try {
            auth.authorize(email, password);
        }
        catch(std::exception &e) {
            auth_failure = true;
            return true;
        }

        auth_complete = true;
        screen.Exit();
        return true;
    };
        
    // Login screen
    ft::Component email_box = custom_ui::custom_component_window(
        ft::text("Email"),
        ft::Input(&email, custom_ui::plain_input())
    );

    ft::InputOption password_opt = custom_ui::plain_input();
    password_opt.password = true;

    ft::Component password_box = custom_ui::custom_component_window(
        ft::text("Password"),
        ft::Input(&password, password_opt)
    );

    ft::Component login_button = ft::Button("Login", authorize, custom_ui::button_rounded());

    ft::Component error_msg = ft::Maybe(ft::Renderer([&] { 
        return ft::text("Login failed! Make sure your login and password are correct. View the backtrace after exiting");
    }), [&]{ return auth_failure; });

    email_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { password_box->TakeFocus(); return true; }
        return false;
    }); 

    password_box |= ft::CatchEvent([&](ft::Event event) {
        if(event == ft::Event::Return) { 
            login_button->TakeFocus();
            authorize();
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
    if(!auth_complete)
        return;
    choose_account_screen(auth);
}

void login::choose_account_screen(const auth& auth_o) {
    // Choose synergia account
    int synergia_account_i = 0;
    auto accounts = auth_o.get_synergia_accounts();
    ft::Component info = ft::Renderer([](){ return ft::text("Please choose a synergia account that you want to use"); });

    ft::Component continue_button = ft::Button("Continue", [&](){ screen.Exit(); }, custom_ui::button_rounded());
    std::vector<std::string> names;
    names.reserve(accounts.size());
    for(const auto& account : accounts)
        names.emplace_back(account.student_name);

    auto account_menu = custom_ui::custom_dropdown(&names, &synergia_account_i);

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
    tab::display_interface(auth_o, accounts[synergia_account_i].login);
}