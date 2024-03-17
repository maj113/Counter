# Faster implementation of `std::count` using AVX2 and SSE 4.2 POPCNT


### Times per 10 iterations, tested on i9-13900H Using GCC 13
| **Method**    | **1Bil** |
|---------------|----------|
| **Standard**  | 1445ms   |
| **Optimized** | 416ms    |
