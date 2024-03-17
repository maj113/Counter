#include <algorithm>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <cstring>
#include <immintrin.h>

using namespace std;
using namespace std::chrono;

static inline uint64_t opt_count(const char* begin, const char* end) noexcept {

    const __m256i avx2_Target = _mm256_set1_epi8(10);
    uint64_t result = 0;

    static __m256i cnk1, cnk2;
    static __m256i cmp1, cmp2;
    static uint32_t msk1, msk2;
    static uint64_t cst;

    for (; begin < end; begin += 64) {
        // Why is lddqu faster than load aligned?
        // supposedly it can prefetch 32 bytes in advance
        cnk1 = _mm256_lddqu_si256((const __m256i*)(begin));
        cnk2 = _mm256_stream_load_si256((const __m256i*)(begin+32));

        cmp1 = _mm256_cmpeq_epi8(cnk1, avx2_Target);
        cmp2 = _mm256_cmpeq_epi8(cnk2, avx2_Target);

        msk1 = _mm256_movemask_epi8(cmp1);
        msk2 = _mm256_movemask_epi8(cmp2);
        // Casting and shifting is faster than 2 popcnt calls
        cst = static_cast<uint64_t>(msk2) << 32;
        result += _mm_popcnt_u64(msk1 | cst);
    }

    return result;
}

int main() {
    const uint64_t dataSize = 1000000000;
    const size_t alignment = 32;

    char* data_unaligned = new char[dataSize + alignment];

    auto raw_address = reinterpret_cast<uintptr_t>(data_unaligned);
    size_t adjustment = alignment - (raw_address % alignment);

    char* data = data_unaligned + adjustment;
    memset(data, '\n', dataSize);
    for (uint64_t i = 0; i < dataSize; i += 2) {
        data[i] = 'x';
    }

    const int numIterations = 10;
    milliseconds durationArr[numIterations];
    milliseconds cumulativeTime = milliseconds(0);

    for (int i = 0; i < numIterations; ++i) {
        auto strt = steady_clock::now();
        uint64_t count = opt_count(data, data + dataSize);
        auto stop = steady_clock::now();
        auto duration = duration_cast<milliseconds>(stop - strt);
        std::cout << "Call " << std::setw(2) << i+1 << " Count: " << count << " " << duration.count() << "ms\n";

        durationArr[i] = duration;
        cumulativeTime += duration;
    }

    cout << "Cumulative Time: " << cumulativeTime.count() << "ms" << endl;

    delete[] data_unaligned;
    return 0;
}