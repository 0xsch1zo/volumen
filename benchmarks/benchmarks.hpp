#pragma once
#include "../src/api.hpp"
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

#define VAR_NAME(x) #x

class benchmarks {
    enum api_test_functions {
        GRADES,
        RECENT_GRADES,
        CATEGORIES,
        COMMENTS,
        EVENTS,
        MESSAGES,
        ANNOUCEMENTS,
        SUBJECTS,
        TIMETABLE,
        USERS
    };

    std::unordered_map<api_test_functions, const std::string> mock_file_name_to_handle_map {
        { GRADES,           "_3.0_Grades" },
        { RECENT_GRADES,    "_3.0_Grades" },
        { CATEGORIES,       "_3.0_Grades_Categories" },
        { COMMENTS,         "_3.0_Grades_Comments" },
        { EVENTS,           "_3.0_HomeWorks" },
        { CATEGORIES,       "_3.0_HomeWorks_Categories" },
        { MESSAGES,         "_3.0_Messages" },
        { ANNOUCEMENTS,     "_3.0_SchoolNotices" },
        { SUBJECTS,         "_3.0_Subjects" },
        { TIMETABLE,        "_3.0_Timetables" },
        { USERS,            "_3.0_Users" }
    };

    void parse_grades_test(api& api_o);
    void parse_recent_grades_test(api& api_o);
    void parse_comment_test(api& api_o);
    void prase_generic_info_by_id_test(api& api_o);
    void parse_events_test(api& api_o);
    void parse_messages_test(api& api_o);
    void parse_annoucements_test(api& api_o);
    void parse_timetable_test(api& api_o);
    static const std::string color(const std::string& text, const std::string& foreground, const std::string& background = BLACK_BACKGROUND);
    void load_mock(api_test_functions test_func, std::ostringstream& os);

public:
    class benchmark {
        std::chrono::_V2::high_resolution_clock::time_point start_time;
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
    authorization::synergia_account_t auth_bench(const std::string& email, const std::string& password);
    void api_bench(authorization::synergia_account_t& synergia_acc);
};