#include "login.hpp"
#include "authorization.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

int main () {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);
    login::ui();
    
    spdlog::dump_backtrace();
}