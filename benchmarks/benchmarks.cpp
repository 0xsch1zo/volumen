#include "benchmarks.hpp"
#include <functional>
#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

const std::string benchmarks::color(const std::string& text, const std::string& foreground, const std::string& background) {
    return (std::string)ESCAPE
        + CSI 
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
    std::cout << color(std::to_string(static_cast<double>(duration) * 1e-3) + "ms", BLACK_FOREGROUND, LIGHT_BLUE_BACKGROUND);
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

authorization::synergia_account_t benchmarks::auth_bench(const std::string& email, const std::string& password) {
    const int ACC_NUM = 0;
    std::function<bool(std::string, std::string)> auth_handle = authorization::authorize;

    const int64_t auth_duration = simple_benchmark(auth_handle, email, password);
    print_info("AUTH");
    print_duration(auth_duration);
    std::cout << '\n';
    std::cout << '\n';

    auto synergia_accounts = authorization::get_synergia_accounts();
    return synergia_accounts[ACC_NUM];
}

void benchmarks::api_bench(authorization::synergia_account_t& synergia_acc) {
    api api_o(synergia_acc);
    const std::function<void()> void_api_functions[] = {
        [&]{ parse_comment_test(api_o);             /* pure */},
        [&]{ prase_generic_info_by_id_test(api_o);  /* pure */},
        [&]{ parse_recent_grades_test(api_o);       /* not pure(4) */},
        [&]{ parse_events_test(api_o);              /* not pure(2) */},
        [&]{ parse_messages_test(api_o);            /* not pure(1) */},
        [&]{ parse_annoucements_test(api_o);        /* not pure(1)*/},
        [&]{ parse_grades_test(api_o);              /* not pure(4) */},
        [&]{ parse_timetable_test(api_o);           /* pure */}
    };

    for(auto& func : void_api_functions)
        func(); 
}

void benchmarks::load_mock(api_test_functions test_func, std::ostringstream& os) {
    std::ifstream get_mock("mocks/" + mock_file_name_to_handle_map[test_func]);
    if(!get_mock.is_open())
        throw std::logic_error("File does not exist");

    os << get_mock.rdbuf();
}

void benchmarks::parse_grades_test(api& api_o) {
    std::ostringstream os;
    load_mock(GRADES, os);
    std::shared_ptr<api::grades_t> grades = std::make_shared<api::grades_t>();
    std::function<void()> handle = [&]{ api_o.parse_grades(os, grades); };

    print_info(VAR_NAME(GRADES));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_recent_grades_test(api& api_o) {
    std::ostringstream os;
    load_mock(RECENT_GRADES, os);
    auto grades = std::make_shared<std::vector<api::grade_t>>();

    std::function<void()> handle = [&]{ api_o.parse_recent_grades(os, grades); };

    print_info(VAR_NAME(RECENT_GRADES));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::prase_generic_info_by_id_test(api& api_o) {
    std::ostringstream os;
    const std::string target = "Categories";
    api::generic_info_id_map ids_and_categories;

    load_mock(CATEGORIES, os);
    std::function<void()> handle = [&]{ api_o.parse_generic_info_by_id(os, target, ids_and_categories); };

    print_info(VAR_NAME(CATEGORIES));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_comment_test(api& api_o) {
    std::ostringstream os;
    api::generic_info_id_map ids_and_comments;
    
    load_mock(COMMENTS, os);
    std::function<void()> handle = [&]{ api_o.parse_comment_by_id(os, ids_and_comments); };

    print_info(VAR_NAME(COMMENTS));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_events_test(api& api_o) {
    std::ostringstream os;
    std::shared_ptr<api::events_t> events = std::make_shared<api::events_t>();

    load_mock(EVENTS, os);
    std::function<void()> handle = [&]{ api_o.parse_events(os, events); };

    print_info(VAR_NAME(EVENTS));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_messages_test(api& api_o) {
    std::ostringstream os;
    api::messages_t msgs;
    msgs.messages = std::make_shared<std::vector<api::message_t>>();


    load_mock(MESSAGES, os);
    std::function<void()> handle = [&]{ api_o.parse_messages(os, msgs.messages); };

    print_info(VAR_NAME(MESSAGES));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_annoucements_test(api& api_o) {
    std::ostringstream os;
    std::shared_ptr<std::vector<api::annoucment_t>> annoucments = std::make_shared<std::vector<api::annoucment_t>>();

    load_mock(ANNOUCEMENTS, os);
    std::function<void()> handle = [&]{ api_o.parse_annoucments(os, annoucments); };

    print_info(VAR_NAME(ANNOUCEMENTS));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::parse_timetable_test(api& api_o) {
    std::ostringstream os;
    std::shared_ptr timetable_struct_p = std::make_shared<api::timetable_t>();
    const int DAY_NUM = 7;

    std::shared_ptr<std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]> 
    timetable(new std::shared_ptr<std::vector<api::lesson_t>>[DAY_NUM]);

    for(int i{}; i < DAY_NUM; i++) {
        timetable[i] = std::make_shared<std::vector<api::lesson_t>>();
    }

    timetable_struct_p->timetable = timetable;

    load_mock(TIMETABLE, os);
    std::function<void()> handle = [&]{ api_o.parse_timetable(os, timetable_struct_p); };

    print_info(VAR_NAME(TIMETABLE));
    print_duration(benchmarks::simple_benchmark(handle));
    std::cout << '\n';
    std::cout << '\n';
}
