#include "defs.hpp"
#include <immintrin.h>
#include <future>
#include <array>


#ifdef AVXDEBUG
inline void p256_hex_8(__m256i in) {
    alignas(16) uint8_t v[32];
    _mm256_store_si256((__m256i*)v, in);
    printf("v32_i8: %x %x %x %x | %x %x %x %x | %x %x %x %x | %x %x %x %x | "
           "%x %x %x %x | %x %x %x %x | %x %x %x %x | %x %x %x %x\n",
           v[0], v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],
           v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15],
           v[16], v[17], v[18], v[19], v[20], v[21], v[22], v[23],
           v[24], v[25], v[26], v[27], v[28], v[29], v[30], v[31]);
}

inline void p256_hex_64(__m256i in) {
    alignas(16) int64_t v[4];
    _mm256_store_si256((__m256i*)v, in);
    printf("v4_i64: %lli %lli %lli %lli | %llx %llx %llx %llx\n",
           v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3]);
}
#endif

// @powturbo's code with slight modifications  
inline int64_t opt_count(const char *s, const char *e, const char c) noexcept {
    const __m256i cv = _mm256_set1_epi8(c), zv = _mm256_setzero_si256();
    __m256i sum = zv, acr0;
    constexpr int16_t acrlimit = 255 * 32;
    const char *pe;

    while (s + acrlimit < e) {
        for (acr0 = zv, pe = s + acrlimit; s < pe; s += 160) {
#ifdef AVXDEBUG // Dump signed 8-bit hex values from accumulators 
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) s)));
            p256_hex_8(acr0);
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 32))));
            p256_hex_8(acr0);
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 64))));
            p256_hex_8(acr0);
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 96))));
            p256_hex_8(acr0);
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 128))));
            p256_hex_8(acr0);
#else 
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) s)));
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 32))));
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 64))));
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 96))));
            acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) (s + 128))));

#endif
        }
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr0, zv));
#ifdef AVXDEBUG
        p256_hex_64(sum);
#endif
    }

    for (acr0 = zv; s + 32 < e; s += 32)
        acr0 = _mm256_sub_epi8(acr0, _mm256_cmpeq_epi8(cv, _mm256_load_si256((const __m256i *) s)));
    sum = _mm256_add_epi64(sum, _mm256_sad_epu8(acr0, zv));

    int64_t count =
        _mm256_extract_epi64(sum, 0)
        + _mm256_extract_epi64(sum, 1)
        + _mm256_extract_epi64(sum, 2)
        + _mm256_extract_epi64(sum, 3);

    // Using != could check outside the boundary
    while(s < e) 
        count += *s++ == c;

    return count;
}

uint64_t counter(const char *begin, const char *end, const char target, bool singleThreaded) noexcept {
    if (singleThreaded)
        return opt_count(begin, end, target);

    const size_t total_length = end - begin;

    // FIXME: Don't multiply by 1000 when used with verifier
    //if (total_length < num_threads * 1000)
    //    return opt_count(begin, end, target);
    
    const size_t chunk_size = (total_length + num_threads - 1) / num_threads;

    std::array<std::future<int64_t>, num_threads> futures;

    int64_t total_count = 0;

    for (unsigned int i = 0; i < num_threads; ++i) {
        const char *chunk_begin = begin + i * chunk_size;
        const char *chunk_end = std::min(end, chunk_begin + chunk_size);

        futures[i] = std::async(std::launch::async, [chunk_begin, chunk_end, target] {
            return opt_count(chunk_begin, chunk_end, target);
        });
    }

    for (auto &future : futures)
        total_count += future.get();

    return total_count;
}

uint64_t counter(const char *begin, const char *end, const int target, bool singleThreaded) noexcept {
    // Horrible code
    if (target >= 0 && target <= 127)
        return counter(begin, end, static_cast<const char>(target), singleThreaded);
    return 0;
}
