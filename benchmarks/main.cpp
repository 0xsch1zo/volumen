#include "benchmarks.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);

    int PATH_ARG_NUM = 1;
    int RUN_COUNT_ARG_NUM = 2;
    const std::string help_msg = "Usage: " + (const std::string)argv[0] + "[path to mocks] [run count]";
    if(argv[PATH_ARG_NUM] == "--help" || argv[PATH_ARG_NUM] == "-h" || argc < 3) {
        std::cerr << help_msg;
        exit(1);
    }

    std::string email, password;
    benchmarks bench(argv[PATH_ARG_NUM]);

    std::cin >> email >> password;
    auto synergia_acc = bench.auth_bench(email, password);
    bench.api_bench(synergia_acc, std::stoi((const std::string)argv[RUN_COUNT_ARG_NUM]));

    spdlog::dump_backtrace();
}