#include <volumen.hpp>
#include <ui/login.hpp>
#include <ui/custom_ui.hpp>
#include <ui/main_ui.hpp>
#include <auth/auth.hpp>
#include <misc/config.hpp>
#include <misc/ssave.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace volumen {


void volumen() {
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
    auth auth_o([&]{
        try {
            login.login_screen(auth_o);
        } catch(error::volumen_exception& e) {
            delete config_p;
            exit(0);
        }
    });

    main_ui main_ui(config_p);
    do {
        auth_o.refresh_api_tokens();
        if(!ssave::exists(auth::login_service_field))
            login.choose_account_screen(auth_o);
    } while(ssave::exists(auth::login_service_field) && main_ui.display_interface(auth_o, ssave::get(auth::login_service_field)));

    delete config_p;
    spdlog::dump_backtrace();
}

} // namespace