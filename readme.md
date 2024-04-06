# Faster implementation of `std::count` using AVX2


### Times per 10 iterations, tested on i9-13900H Using GCC 13
| **Method**    | **1Bil** | **500M** | **250M** |  **10M** |  **2M**  |
|---------------|----------|----------|----------|----------|----------|
| **Standard**  |  1358ms  |   684ms  |   342ms  | 13606µs  |  2601µs  |
| **Optimized** |   376ms  |   179ms  |   94ms   |  1901µs  |   292µs  |
| **Multi Opt** |   229ms  |   111ms  |   55ms   |  6963µs  |  5980µs  |
