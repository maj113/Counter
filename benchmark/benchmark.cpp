#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "defs.hpp"

using namespace std;
using namespace chrono;

void printTime(const nanoseconds &duration) {
    double time = duration.count();
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

uint64_t bench(const char *data, uint64_t dataSize, uint64_t numIterations, bool useOptimized, bool singleThreaded) {
    auto cumulativeTime = nanoseconds(0);
    string methodName = useOptimized ? "Opt" : "Std";
    string threadingMode = singleThreaded ? "Single Threaded" : "Multi Threaded";
    cout << "Benchmarking " << threadingMode << " " << methodName << ":\n";
    for (uint64_t i = 0; i < numIterations; ++i) {
        auto strt = high_resolution_clock::now();
        uint64_t count = useOptimized
                    ? opt_count_parallel(
                        data, data + dataSize, 10, singleThreaded)
                    : std::count(data, data + dataSize, 10);
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

int main() {
    constexpr uint64_t dataSize = 10000000000;
    constexpr size_t alignment = 32;

    char *data_unaligned = new char[dataSize + alignment];

    auto raw_address = reinterpret_cast<uintptr_t>(data_unaligned);
    size_t adjustment = alignment - (raw_address % alignment);

    char *data = data_unaligned + adjustment;
    memset(data, '\n', dataSize);
    for (uint64_t i = 0; i < dataSize; i += 2)
        data[i] = 'x';

    constexpr uint64_t numIterations = 10;

    uint64_t opt_single_cumulative = bench(data, dataSize, numIterations, true, true);
    uint64_t opt_multi_cumulative = bench(data, dataSize, numIterations, true, false);
    uint64_t std_single_cumulative = bench(data, dataSize, numIterations, false, true);

    double improvement_single = ((double) std_single_cumulative - (double) opt_single_cumulative) / (double)
                                std_single_cumulative * 100.0;
    double improvement_multi = ((double) std_single_cumulative - (double) opt_multi_cumulative) / (double)
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

    delete[] data_unaligned;
    return 0;
}
