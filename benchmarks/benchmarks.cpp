#include "benchmarks.hpp"
#include <functional>
#include <iostream>
#include <thread>

const std::string benchmarks::color(const std::string& text, const std::string& foreground, const std::string& background) {
    return (std::string)ESCAPE + 
        CSI 
            + foreground 
        + DELIMINATOR
            + background
        + TERMINATOR
            + text
        + ESCAPE
        + CSI 
        + RESET;
}

template<typename R, typename... Args>
const int64_t benchmarks::simple_benchmark(std::function<R(Args...)> func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();

    func(args...);

    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void benchmarks::print_duration(int64_t duration) {
    std::cout << color(std::to_string(duration), BLACK_FOREGROUND, LIGHT_BLUE_BACKGROUND);
}

void benchmarks::print_info(std::string text) {
    std::cout << color(text, LIGHT_BLUE_FOREGROUND, DARK_GRAY_BACKGROUND);
}

void benchmarks::benchmark::start() {
    start_time = std::chrono::high_resolution_clock::now();
}

void benchmarks::benchmark::pause() {
    auto pause = std::chrono::high_resolution_clock::now();
    total_time += std::chrono::duration_cast<std::chrono::microseconds>(pause - start_time).count();
}

int64_t benchmarks::benchmark::end_bench() {
    return total_time;
}

void benchmarks::benchmark::reset() {
    total_time = 0;
}

/*authorization::synergia_account_t& benchmarks::auth_bench(const std::string& email, const std::string& password) {
    const int ACC_NUM = 0;
    std::function<bool(std::string, std::string)> auth_handle = authorization::authorize;

    const int64_t auth_duration = simple_benchmark(auth_handle, email, password);
    print_info("Auth ");
    print_duration(auth_duration);
    std::cout << '\n';

    auto synergia_accounts = authorization::get_synergia_accounts();
    return synergia_accounts[ACC_NUM];
}

void benchmarks::api_bench(authorization::synergia_account_t& synergia_acc) {
    api api_o(synergia_acc);
    const std::function<void()> void_api_functions[] = {
        [&]{ api_o.get_annoucments(); },
        [&]{ api_o.get_events(); },
        [&]{ api_o.get_grades(); },
        [&]{ api_o.get_messages(); },
        [&]{ api_o.get_recent_grades(); },
        [&]{ api_o.get_today(); }
    };

    const std::string void_func_names[] = {
        "api::get_annoucments ",
        "api::get_events ",
        "api::get_grades ",
        "api::get_messages ",
        "api::get_recent_grades ",
        "api::get_today "
    };
    
    for(int i{}; i < sizeof(void_api_functions) / sizeof(void_api_functions[0]); i++) {
        const int64_t api_bench_duration = benchmarks::simple_benchmark(void_api_functions[i]);
        print_info(void_func_names[i]);
        print_duration(api_bench_duration);
    }
}*/