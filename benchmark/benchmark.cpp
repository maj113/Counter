#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <algorithm>
#include <cstring>
#include "../defs.hpp"

using namespace std;
using namespace chrono;

void printTime(const nanoseconds &duration) {
    double time = static_cast<double>(duration.count());
    std::string_view unit;
    if (time >= 1e9) {
        time *= 1e-9;
        unit = "s";
    } else if (time >= 1e6) {
        time *= 1e-6;
        unit = "ms";
    } else if (time >= 1e3) {
        time *= 1e-3;
        unit = "us";
    } else {
        unit = "ns";
    }
    cout << fixed << setprecision(2) << time << unit;
}

uint64_t bench(const char *data, uint64_t dataSize, uint8_t numIterations, int method, bool stressTest = false) noexcept {
    
    if (stressTest) {
        uint64_t result = 0;
        for (uint8_t i = 0; i < numIterations * 100; ++i)
            result += counter(data, data + dataSize, 10, false);
        cout << result << endl;
        return result;
    }

    auto printMethod = [](int method) -> void {
        switch (method) {
            case COUNT_STANDARD: cout << "Single Threaded Std:\n"; break;
            case COUNT_OPTIMIZED: cout << "Single Threaded Opt:\n"; break;
            case COUNT_OPTIMIZED_PARALLEL: cout << "Multi Threaded Opt:\n"; break;
            default: cerr << "Invalid method specified!\n"; return;
        }
    };

    auto setCount = [&data, &dataSize](int method) -> uint64_t {
        switch (method) {
            case COUNT_STANDARD:
                return std::count(data, data + dataSize, 10);
            case COUNT_OPTIMIZED:
                return counter(data, data + dataSize, 10, true);
            case COUNT_OPTIMIZED_PARALLEL:
                return counter(data, data + dataSize, 10, false);
            default:
                cerr << "Invalid method specified!\n";
                return 0;
        }
    };

    printMethod(method);

    auto cumulativeTime = nanoseconds(0);

    for (uint64_t i = 0; i < numIterations; ++i) {
        auto strt = high_resolution_clock::now();
        uint64_t count = setCount(method);
        auto stop = high_resolution_clock::now();
        auto duration = stop - strt;
        cumulativeTime += duration;

        cout << "  Call " << setw(2) << i + 1 << ": " << "Count: " << count << ", ";
        printTime(duration);
        cout << "\n";
    }

    cout << "  Cumulative Time: ";
    printTime(cumulativeTime);
    cout << "\n";

    double totalBytesProcessed = static_cast<double>(dataSize) * numIterations;
    double totalSecs = cumulativeTime.count() / 1e9;
    double throughputGBps = (totalBytesProcessed / totalSecs) / (1024 * 1024 * 1024);

    cout << "  Throughput: " << throughputGBps << " GB/s\n" << endl;

    return cumulativeTime.count();
}

int main(int argc, char* argv[]) {
    constexpr uint64_t dataSize = 10000000000;
    constexpr size_t alignment = 32;
    constexpr uint8_t numIterations = 10;

    char* data_unaligned = new char[dataSize + alignment];

    auto raw_address = reinterpret_cast<uintptr_t>(data_unaligned);
    size_t adjustment = alignment - (raw_address % alignment);
    char* data = data_unaligned + adjustment;
    
    memset(data, '\n', dataSize);
    
    for (uint64_t i = 0; i < dataSize; i += 2)
        data[i] = 'x';

    if (argc > 1 && strcmp(argv[1], "--stress") == 0) {
        for (uint8_t i = 0; i < numIterations; ++i)
            bench(data, dataSize, numIterations, COUNT_OPTIMIZED_PARALLEL,  true);
        cout << "Stress test done!" << endl;
    
    } else {
        uint64_t opt_single_cumulative = bench(data, dataSize, numIterations, COUNT_OPTIMIZED);
        uint64_t opt_multi_cumulative = bench(data, dataSize, numIterations, COUNT_OPTIMIZED_PARALLEL);
        uint64_t std_single_cumulative = bench(data, dataSize, numIterations, COUNT_STANDARD);

        double improvement_single = ((double)std_single_cumulative - (double)opt_single_cumulative) / (double)
                                    std_single_cumulative * 100.0;
        double improvement_multi = ((double)std_single_cumulative - (double)opt_multi_cumulative) / (double)
                                   std_single_cumulative * 100.0;

        cout << "Improvement over std::count: single threaded: " << fixed << setprecision(2)
             << (improvement_single >= 0 ? "" : "-") << abs(improvement_single)
             << "% multi threaded: " << fixed << setprecision(2)
             << (improvement_multi >= 0 ? "" : "-") << abs(improvement_multi) << "%" << endl;

        cout << "Times faster over std::count: single threaded: " << fixed << setprecision(2)
             << ((double)std_single_cumulative / (double)opt_single_cumulative >= 0 ? "" : "-")
             << abs((double)std_single_cumulative / (double)opt_single_cumulative)
             << "x multi threaded: " << fixed << setprecision(2)
             << ((double)std_single_cumulative / (double)opt_multi_cumulative >= 0 ? "" : "-")
             << abs((double)std_single_cumulative / (double)opt_multi_cumulative) << "x" << endl;
    }

    delete[] data_unaligned;
    return 0;
}
