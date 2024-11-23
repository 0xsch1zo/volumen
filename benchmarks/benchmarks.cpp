#include "benchmarks.hpp"
#include <functional>
#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>
#include <misc/utils.hpp>

namespace fs = std::filesystem;

benchmarks::benchmarks(const std::string& mocks) : mock_dir(mocks) {}

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

void benchmarks::auth_bench(const std::string& email, const std::string& password, auth& auth_o) {
    const int ACC_NUM = 0;
    std::function<void()> auth_handle = [&]{ auth_o.authorize(email, password); };

    const int64_t auth_duration = simple_benchmark(auth_handle);
    print_info("AUTH");
    print_duration(auth_duration);
    std::cout << '\n';
    std::cout << '\n';
}

void benchmarks::api_bench(auth& auth_o, int run_count) {
    const int TEST_LOGIN_INDEX = 0;

    api api_o(
        auth_o, 
        auth_o.get_synergia_accounts()[TEST_LOGIN_INDEX].login
    );

    const std::function<test_result()> void_api_functions[] = {
        [&]{ return parse_comment_test(api_o);             /* pure */},
        [&]{ return prase_generic_info_by_id_test(api_o);  /* pure */},
        [&]{ return parse_grades_unstructured_test(api_o);       /* not pure(4) */},
        [&]{ return parse_events_test(api_o);              /* not pure(2) */},
        [&]{ return parse_messages_test(api_o);            /* not pure(1) */},
        [&]{ return parse_annoucements_test(api_o);        /* not pure(1)*/},
        [&]{ return parse_grades_test(api_o);              /* not pure(4) */},
        [&]{ return parse_timetable_test(api_o);           /* pure */}
    };

    for(auto& func : void_api_functions) {
        std::string test_name;
        int64_t total_duration{};
        for(int i = 0; i < run_count; i++) {
            test_result res = func(); 
            test_name = res.test_function;
            total_duration += res.duration;
        }

        print_info(test_name);
        print_duration(total_duration / (int64_t)run_count);
        std::cout << std::endl << std::endl;
    }
}

std::string benchmarks::load_mock(api_test_functions test_func) {
    std::ifstream get_mock(mock_dir + mock_file_name_to_handle_map[test_func]);
    if(!get_mock.is_open())
        throw std::logic_error("File does not exist");

    std::ostringstream os;
    os << get_mock.rdbuf();
    return os.str();
}

benchmarks::test_result benchmarks::parse_grades_test(api& api_o) {
    api::grades_t grades_o;
    std::function<void()> handle = [&]{ api_o.parse_grades(load_mock(GRADES), grades_o); };

    return { VAR_NAME(GRADES), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_grades_unstructured_test(api& api_o) {
    api::grades_unstructured_t grades;

    std::function<void()> handle = [&]{ api_o.parse_grades_unstructured(load_mock(RECENT_GRADES), grades); };

    return { VAR_NAME(RECENT_GRADES), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::prase_generic_info_by_id_test(api& api_o) {
    const std::string target = "Categories";
    api::generic_info_id_map ids_and_categories;

    std::function<void()> handle = [&]{ api_o.parse_generic_info_by_id(load_mock(GENERIC), target, ids_and_categories); };

    return { VAR_NAME(GENERIC), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_comment_test(api& api_o) {
    api::generic_info_id_map ids_and_comments;
    
    std::function<void()> handle = [&]{ api_o.parse_comment_by_id(load_mock(COMMENTS), ids_and_comments); };

    return { VAR_NAME(COMMENTS), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_events_test(api& api_o) {
    api::events_t events_o;

    std::function<void()> handle = [&]{ api_o.parse_events(load_mock(EVENTS), events_o); };

    return { VAR_NAME(EVENTS), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_messages_test(api& api_o) {
    api::messages_t messages_o;

    std::function<void()> handle = [&]{ api_o.parse_messages(load_mock(MESSAGES), messages_o.recieved); };

    return { VAR_NAME(MESSAGES), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_annoucements_test(api& api_o) {
    api::annoucements_t annoucments;

    std::function<void()> handle = [&]{ api_o.parse_annoucments(load_mock(ANNOUCEMENTS), annoucments); };

    return { VAR_NAME(ANNOUCEMENTS), benchmarks::simple_benchmark(handle) };
}

benchmarks::test_result benchmarks::parse_timetable_test(api& api_o) {
    api::timetable_t timetable_o;

    std::function<void()> handle = [&]{ api_o.parse_timetable(load_mock(TIMETABLE), timetable_o); };

    return { VAR_NAME(TIMETABLE), benchmarks::simple_benchmark(handle) };
}
