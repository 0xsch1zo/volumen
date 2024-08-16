#include "login.hpp"
#include <sstream>
#include <string>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

ft::Elements login::split(std::string text) {
        ft::Elements output;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line, '\n'))
            output.push_back(ft::text(line));
        return output;
}

void login::ui() {
        std::string login;
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

        ftxui::Component login_box = ftxui::Input(&login);

        ftxui::InputOption password_opt;
        password_opt.password = true;
        ftxui::Component password_box = ftxui::Input(&password, password_opt);

        login_box |= ftxui::CatchEvent([&](ftxui::Event event) {
            if(event == ftxui::Event::Return) { password_box->TakeFocus(); return true; }
            return false;
        }); 

        auto component = ftxui::Container::Vertical({
            login_box,
            password_box
        });

        auto renderer = ftxui::Renderer(component, [&] {
            return ftxui::vbox({
                    ftxui::vbox(split(splash)) | ftxui::hcenter,
                    ftxui::vbox({
                        ftxui::window(ftxui::text("Login"), login_box->Render()) 
                            | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40),

                        ftxui::window(ftxui::text("Password"), password_box->Render())
                            | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40)
                        })  | ftxui::hcenter
                })  |
                ftxui::vcenter;
        });

        auto screen = ftxui::ScreenInteractive::Fullscreen();
        screen.Loop(renderer);
}