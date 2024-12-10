#pragma once
#include <api/api.hpp>
#include <auth/auth.hpp>
#include <string>
#include <functional>
#include <unordered_map>

#define ESCAPE                  "\033"
#define CSI                     "["
#define DELIMINATOR             ";"
#define TERMINATOR              "m"
#define LIGHT_BLUE_FOREGROUND   "36"
#define LIGHT_BLUE_BACKGROUND   "46"
#define DARK_GRAY_FOREGROUND    "90"
#define DARK_GRAY_BACKGROUND    "100"
#define BLACK_FOREGROUND        "30"
#define BLACK_BACKGROUND        "40"
#define RESET                   "0m"

class benchmarks {
    typedef enum api_test_functions {
        GRADES,
        RECENT_GRADES,
        GENERIC,
        COMMENTS,
        EVENTS,
        MESSAGES,
        ANNOUCEMENTS,
        SUBJECTS,
        TIMETABLE,
        USERS
    };

    struct test_result {
        std::string test_function;
        int64_t duration;
    };

    std::unordered_map<api_test_functions, const std::string> mock_file_name_to_handle_map {
        { GRADES,           "_3.0_Grades" },
        { RECENT_GRADES,    "_3.0_Grades" },
        //{ CATEGORIES,       "_3.0_Grades_Categories" },
        { COMMENTS,         "_3.0_Grades_Comments" },
        { EVENTS,           "_3.0_HomeWorks" },
        { GENERIC,          "_3.0_HomeWorks_Categories" },
        { MESSAGES,         "_3.0_Messages" },
        { ANNOUCEMENTS,     "_3.0_SchoolNotices" },
        { SUBJECTS,         "_3.0_Subjects" },
        { TIMETABLE,        "_3.0_Timetables" },
        { USERS,            "_3.0_Users" }
    };

    std::string mock_dir;

    test_result parse_grades_test(api& api_o);
    test_result parse_grades_unstructured_test(api& api_o);
    test_result parse_comment_test(api& api_o);
    test_result prase_generic_info_by_id_test(api& api_o);
    test_result parse_events_test(api& api_o);
    test_result parse_messages_test(api& api_o);
    test_result parse_annoucements_test(api& api_o);
    test_result parse_timetable_test(api& api_o);
    static const std::string color(const std::string& text, const std::string& foreground, const std::string& background = BLACK_BACKGROUND);
    std::string load_mock(api_test_functions test_func);

public:
    benchmarks(const std::string& mocks);
    class benchmark {
        std::chrono::high_resolution_clock::time_point start_time;
        int64_t total_time;
    public:
        void start();
        void pause();
        int64_t end_bench();
        void reset();
    };

private:
    template<typename R, typename... Args>
    static const int64_t simple_benchmark(std::function<R(Args...)> func, Args... args);
    static void print_duration(int64_t duration);
    static void print_info(std::string text);
public:
    void auth_bench(const std::string& email, const std::string& password, auth& auth_o);
    void api_bench(auth& auth_o, int run_count);
};