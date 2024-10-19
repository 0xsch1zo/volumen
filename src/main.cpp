#include "login.hpp"
#include "api.hpp"
#include "config.hpp"
#include "custom_ui.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

// TODO: make loggger  display messages only in backtrace
int main () {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);

    std::unique_ptr<config> config_p;

    try {
        config_p = std::make_unique<config>();
    } catch(std::exception& e) {
        spdlog::error(e.what());
        exit(1);
    }

    custom_ui::init(config_p.get());
    login login(config_p.get());
    login.login_screen();

    spdlog::dump_backtrace();
    return 0;
}