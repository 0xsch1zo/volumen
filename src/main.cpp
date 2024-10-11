#include "login.hpp"
#include "api.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

// TODO: make loggger  display messages only in backtrace
int main () {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);

    login login;
    login.login_screen();

    spdlog::dump_backtrace();
    return 0;
}