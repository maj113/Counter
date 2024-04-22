#include <iostream>
#include <algorithm>
#include <array>
#include <cstring>
#include "../defs.hpp"
#include <cassert>

int main() {
    std::array data_sizes{
        100, 500, 1000, 5000, 10000, 20000, 50000, 75000, 77777, 80001, 85013, 90017,
        100000, 123456, 250000, 500000, 750000, 800001, 850005, 900009,
        1000000, 1500000, 2000000, 3000000, 4000000, 5000000,
        7500000, 10000000, 15000000, 20000000, 30000000, 40000000, 50000000,
        77777777, 88888888, 100000000, 123456789, 125000000, 150000000, 987654321, 1500000000
    };
    constexpr char target = 'x', filler = 'a';

    for (uint64_t size : data_sizes) {
        char* rmem = new char[size + 32];
        
        uintptr_t raddr = reinterpret_cast<uintptr_t>(rmem);
        uintptr_t misalignment = raddr % 32;
        size_t adjustment = misalignment == 0 ? 0 : 32 - misalignment;
        char* data = rmem + adjustment;
        
        std::memset(data, filler, size);

        for (uint64_t count = 1; count <= size; count *= 3) { 
            for (uint64_t i = 0; i < count; ++i) {
                data[i] = target;
            }

            std::cout << "Array size: " << size << ", Number of '" << target << "': " << count << "\n";

            uint64_t rParallel = opt_count_parallel(data, data + size, target, false);
            uint64_t rSingle = opt_count_parallel(data, data + size, target,  true);
            uint64_t rStandard = std::count(data, data + size, target);

            std::cout << "opt_parallel count: " << rParallel << ", opt_single count: " << rSingle
                    << ", std::count count: " << rStandard << "\n";

            if (rParallel != count || rSingle != count || rStandard != count) {
                std::cerr << "\033[31m" << "Counts for '" << target << "' do not match the expected amount:\n";
                if (rParallel != count) {
                    std::cerr << "  - Parallel count: Expected " << count << ", Actual " << rParallel << "\n";
                }
                if (rSingle != count) {
                    std::cerr << "  - Single count: Expected " << count << ", Actual " << rSingle << "\n";
                }
                if (rStandard != count) {
                    std::cerr << "  - Standard count: Expected " << count << ", Actual " << rStandard << "\n";
                }
                std::cerr << "\033[0m";
                assert((false && "Incorrect count!!!"));
            }
            std::cout << "------------------------------------------------\n";
        }
        std::cout << "\033[H\033[2J";
        delete[] rmem;
    }
    return 0;
}