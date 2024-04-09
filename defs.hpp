#pragma once

#include <cstdint>

/**
 * @brief Counts occurrences of a target character in a given range of characters using parallel execution.
 *
 * This function divides the input range into chunks and utilizes multiple threads to count occurrences of
 * the target character in each chunk. The results are combined to obtain the total count.
 *
 * @param begin Pointer to the beginning of the character sequence.
 * @param end Pointer to the end of the character sequence (one past the last element).
 * @param target The character to be counted.
 * @param singleThreaded Optional. If set to true, forces the function to execute in a single thread.
 *                      Defaults to false.
 * @return The total count of occurrences of the target character in the specified range.
 */
uint64_t opt_count_parallel(const char *begin, const char *end, char target, bool singleThreaded) noexcept;

uint64_t opt_count_parallel(const char *begin, const char *end, int target, bool singleThreaded) noexcept;
