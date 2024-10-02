#include "benchmarks.hpp"
#include <iostream>

int main() {
    std::string email, password;
    benchmarks bench;

    std::cin >> email >> password;
    auto synergia_acc = bench.auth_bench(email, password);
    bench.api_bench(synergia_acc);
}