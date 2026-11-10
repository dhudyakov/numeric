#include "itoa.hpp"
#include <algorithm>

template <typename T, size_t N>
bool verify(const T (&test_cases)[N])
{
    char buffer[30];
    bool all_passed = true;

    for (auto val : test_cases)
    {
        std::fill(std::begin(buffer), std::end(buffer), '#');
        uint32_t len = itoa_fast(val, buffer);

        std::string_view out_str(buffer, len);
        std::string expected_str = std::to_string(val);

        bool match = out_str == expected_str;
        if (!match)
            all_passed = false;

        std::cout << "Input:    " << val << "\n";
        std::cout << "Output:   \"" << out_str << "\"\n";
        std::cout << "Length:   " << len << "\n";
        std::cout << "Status:   " << (match ? "✅ PASSED" : "❌ FAILED (Expected: " + expected_str + ")") << "\n";
        std::cout << "------------------------------------------------------------\n";
    }
    return all_passed;
}

int main()
{
    bool all_passed = true;
    // Comprehensive array of edge cases and mathematical transition boundaries
    uint64_t test_cases[] = 
    {
        0ULL, // Absolute minimum (Verifies the branchless 0x80000 bit fallback)
        1ULL, // Single digit transition
        9ULL,
        10ULL, // Two digit power transition
        99ULL,
        100ULL,                  // Three digit power transition
        123ULL,                  // Basic verification check
        99999999ULL,             // 8 digits: Maximum boundary for low_chunk
        100000000ULL,            // 9 digits: Entry threshold into mid_chunk
        1234567890ULL,           // Standard random 10-digit number
        9999999999999999ULL,     // 16 digits: Maximum boundary for mid_chunk
        10000000000000000ULL,    // 17 digits: Entry threshold into top_4_chunk
        10000000000000001ULL,    // Tests non-zero lower paths paired with large top chunks
        18446744073709551614ULL, // UINT64_MAX - 1
        18446744073709551615ULL  // Absolute 64-bit maximum (20 full text digits)
    };
    all_passed &= verify(test_cases);

    int64_t signed_test_cases[] = 
    {
        // --- 1. Zero and Small Boundaries ---
        0,  // The absolute fallback clamp case (Must not inject '-')
        -1, // The absolute minimum length negative value (-1 bit mask tracking)
        1,  // Smallest positive non-zero digit
        -9, // Single-digit negative limit
        9,  // Single-digit positive limit

        // --- 2. Single-to-Double Digit Lane Transitions ---
        -10,  // Two-digit negative power transition
        10,   // Two-digit positive power transition
        -99,  // Lower bound of two-digit negative space
        -100, // Threshold crossing into 3 digits

        // --- 3. Chunk & Multiplier Boundary Crossings (Daniel Lemire Multiplicative Inverses) ---
        -99999999,          // Maximum 8-digit boundary for the low_chunk calculations
        -100000000,         // 9 digits: Entry threshold shifting text bytes into mid_chunk space
        100000000,          // Matching positive mid-chunk crossing
        -9999999999999999,  // 16 digits: High boundary for mid_chunk execution ports
        -10000000000000000, // 17 digits: Precise threshold entry into high_val multiplier
        10000000000000000,  // Matching positive high-val threshold

        // --- 4. The -2 Two's Complement Threshold Check ---
        -2, // Tests the exact input that triggered the raw bit wrapper overflow

        // --- 5. Absolute Hardware Limits (INT64_MAX / INT64_MIN) ---
        9223372036854775807,     // INT64_MAX: Maximum possible 19-digit positive int64_t
        -9223372036854775807,    // INT64_MIN + 1: Maximum length 19-digit negative value
        -9223372036854775807 - 1 // INT64_MIN: The structural edge case.
                                 // Highly critical: triggers maximum width (20 total text characters "-9223372036854775808")
                                 // and ensures magnitude = (magnitude ^ sign_flip) - sign_flip handles two's complement negation without undefined bugs.
    };
    all_passed &= verify(signed_test_cases);

    // Comprehensive test cases for unsigned 32-bit integers
    uint32_t uint32_test_cases[] = 
    {
        0U, // Absolute minimum fallback clamp case
        1U, // Single-digit positive transition
        9U,
        10U, // Two-digit power transition
        99U,
        100U,        // Three-digit power transition
        123U,        // Basic verification check
        9999999U,    // 7 digits: High boundary for lower chunk math
        99999999U,   // 8 digits: Absolute maximum limit of the low_val block
        100000000U,  // 9 digits: Entry threshold into high_val multiplier space
        123456789U,  // Standard random 9-digit layout
        4294967294U, // UINT32_MAX - 1
        4294967295U  // Absolute 32-bit unsigned maximum (10 full text digits)
    };
    all_passed &= verify(uint32_test_cases);

    // Comprehensive test cases for signed 32-bit integers
    int32_t int32_test_cases[] = {
        // --- 1. Zero and Small Boundaries ---
        0,  // Fallback clamp case (Must not inject '-')
        -1, // Smallest length negative value (-1 bit mask check)
        1,  // Smallest positive non-zero digit
        -9, // Single-digit negative limit
        9,  // Single-digit positive limit

        // --- 2. Digit Lane Transitions ---
        -10,  // Two-digit negative power transition
        10,   // Two-digit positive power transition
        -99,  // Lower bound of two-digit negative space
        -100, // Threshold crossing into 3 digits

        // --- 3. Chunk & Multiplier Boundary Crossings ---
        -99999999,  // Maximum 8-digit boundary for the low_val calculations
        -100000000, // 9 digits: Entry threshold shifting text bytes into high_val space
        100000000,  // Matching positive threshold crossing

        // --- 4. Absolute Hardware Limits (INT32_MAX / INT32_MIN) ---
        2147483647,     // INT32_MAX: Maximum possible 10-digit positive int32_t
        -2147483647,    // INT32_MIN + 1: Maximum length 10-digit negative value
        -2147483647 - 1 // INT32_MIN (-2147483648): The structural edge case.
                        // Triggers maximum length (11 total characters) and checks
                        // branchless magnitude negation without undefined overflow bugs.
    };
    all_passed &= verify(int32_test_cases);

    if (all_passed)
    {
        std::cout << " 🎉 ALL TEST CASES PASSED PERFECTLY!\n";
    }
    else
    {
        std::cout << " 🚨 SOME TEST CASES FAILED.\n";
    }

    return 0;
}
