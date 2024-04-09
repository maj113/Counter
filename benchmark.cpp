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

uint64_t bench(const char *data, uint64_t dataSize, uint64_t numIterations, bool useOptimized, bool singleThreaded) {
    milliseconds cumulativeTime = milliseconds(0);
    string methodName = useOptimized ? "Opt" : "Std";
    string threadingMode = singleThreaded ? "Single Threaded" : "Multi Threaded";
    cout << "Benchmarking " << threadingMode << " " << methodName << ":\n";
    for (uint64_t i = 0; i < numIterations; ++i) {
        auto strt = steady_clock::now();
        uint64_t count = useOptimized
                             ? opt_count_parallel(
                                 data, data + dataSize, 10, singleThreaded)
                             : std::count(data, data + dataSize, 10);
        auto stop = steady_clock::now();
        auto duration = duration_cast<milliseconds>(stop - strt);
        cumulativeTime += duration;

        cout << "  Call " << setw(2) << i + 1 << ": " << "Count: " << count << ", " << duration.count() << "ms\n";
    }

    cout << "  Cumulative Time: " << cumulativeTime.count() << "ms\n";

    double totalBytesProcessed = static_cast<double>(dataSize) * numIterations;
    double totalms = cumulativeTime.count() / 1000.0;
    double throughputGBps = (totalBytesProcessed / totalms) / (1024 * 1024 * 1024);

    cout << "  Throughput: " << throughputGBps << " GB/s\n" << endl;

    return cumulativeTime.count();
}

int main() {
    constexpr uint64_t dataSize = 1000000000;
    constexpr size_t alignment = 32;

    char *data_unaligned = new char[dataSize + alignment];

    auto raw_address = reinterpret_cast<uintptr_t>(data_unaligned);
    size_t adjustment = alignment - (raw_address % alignment);

    char *data = data_unaligned + adjustment;
    memset(data, '\n', dataSize);
    for (auto i = 0; i < dataSize; i += 2)
        data[i] = 'x';

    constexpr uint64_t numIterations = 10;

    uint64_t opt_single_cumulative = bench(data, dataSize, numIterations, true, true);
    uint64_t opt_multi_cumulative = bench(data, dataSize, numIterations, true, false);
    uint64_t std_single_cumulative = bench(data, dataSize, numIterations, false, true);

    double improvement_single = ((double) std_single_cumulative - (double) opt_single_cumulative) / (double)
                                std_single_cumulative * 100.0;
    double improvement_multi = ((double) std_single_cumulative - (double) opt_multi_cumulative) / (double)
                               std_single_cumulative * 100.0;

    cout << "Improvement over std::count: single threaded: " << fixed << setprecision(2) << abs(improvement_single) <<
            "% multi threaded: " << fixed << setprecision(2) << abs(improvement_multi) << "%" << endl;

    cout << "Times faster over std::count: single threaded: " << fixed << setprecision(2) << (double)
            std_single_cumulative / (double) opt_single_cumulative << "x multi threaded: " << fixed << setprecision(2)
            << (double) std_single_cumulative / (double) opt_multi_cumulative << "x" << endl;

    delete[] data_unaligned;
    return 0;
}
