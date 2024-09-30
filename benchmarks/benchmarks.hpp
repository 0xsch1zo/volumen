#include "../src/api.hpp"
#include <string>
#include <functional>

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
    static const std::string color(const std::string& text, const std::string& foreground, const std::string& background = BLACK_BACKGROUND);
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

    template<typename R, typename... Args>
    static const int64_t simple_benchmark(std::function<R(Args...)> func, Args... args);
    static void print_duration(int64_t duration);
    static void print_info(std::string text);
};