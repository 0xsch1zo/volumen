#include "login.hpp"
#include "auth.hpp"
#include "config.hpp"
#include "custom_ui.hpp"
#include "ssave.hpp"
#include "tab.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

// TODO: make loggger  display messages only in backtrace
int main () {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);
    
    config* config_p;

    try {
        config_p = new config();
    } catch(std::exception& e) {
        spdlog::error(e.what());
        exit(1);
    }
    
    custom_ui::init(config_p);
    login login(config_p);
    auth auth_o([&]{ login.login_screen(auth_o); });

    if(auth_o.refresh_api_tokens()) {
        if(!ssave::exists("login"))
            login.choose_account_screen(auth_o);
        
        tab tab_o(config_p);
        tab_o.display_interface(auth_o, ssave::get("login"));
    }

    delete config_p;
    spdlog::dump_backtrace();
    return 0;
}