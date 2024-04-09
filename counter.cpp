#include "defs.hpp"
#include <immintrin.h>
#include <cstdint>
#include <vector>
#include <future>
#include <iostream>

// @powturbo's code with slight modifications  
inline uint64_t opt_count(const char *s, const char *e, const char c) {
    __m256i cv = _mm256_set1_epi8(c), zv = _mm256_setzero_si256(), sum = zv, acr0, acr1, acr2, acr3;

    const char *p, *pe;
    for (p = s; p != e - ((e - s) % (252 * 32));) {
        for (acr0 = acr1 = acr2 = acr3 = zv, pe = p + 252 * 32; p != pe; p += 128) {
            // lddqu and stream_load would be faster on MSVC
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) p)));
            acr1 = _mm256_sub_epi8(acr1, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (p + 32))));
            acr2 = _mm256_sub_epi8(acr2, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (p + 64))));
            acr3 = _mm256_sub_epi8(acr3, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (p + 96))));
            // MSVC might not compile the code with cast to void
            _mm_prefetch((const void *) (p + 1024), _MM_HINT_T0);
        }
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr0, zv));
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr1, zv));
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr2, zv));
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr3, zv));
    }

    for (acr0 = zv; p < e; p += 32)
        acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) p)));
    sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr0, zv));

    size_t count =
            _mm256_extract_epi64(sum, 0)
            + _mm256_extract_epi64(sum, 1)
            + _mm256_extract_epi64(sum, 2)
            + _mm256_extract_epi64(sum, 3);

    return count;
}

uint64_t opt_count_parallel(const char *begin, const char *end, const char target, bool singleThreaded) noexcept {
    const unsigned int num_threads = std::thread::hardware_concurrency();
    const size_t total_length = end - begin;

    if ((total_length < num_threads * 2) or singleThreaded)
        return opt_count(begin, end, target);
    // TODO: better/more precise chunk size calculation needed
    const size_t chunk_size = (total_length + num_threads - 1) / num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    uint64_t total_count = 0;

    for (size_t i = 0; i < num_threads; ++i) {
        const char *chunk_begin = begin + (i * chunk_size);
        const char *chunk_end = std::min(end, chunk_begin + chunk_size);

        threads.emplace_back([&total_count, chunk_begin, chunk_end, target]() {
            total_count += opt_count(chunk_begin, chunk_end, target);
        });
    }

    for (auto &thread: threads)
        thread.join();

    return total_count;
}

uint64_t opt_count_parallel(const char *begin, const char *end, const int target, bool singleThreaded) noexcept {
    // Horrible code
    if (0 <= target <= 127)
        return opt_count_parallel(begin, end, (const char) target, singleThreaded);
    return 0;
}
