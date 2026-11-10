#pragma once
#include <immintrin.h>
#include <x86intrin.h>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <stdint.h>
#include <bit>

inline __m256i generate_digits(uint64_t value)
{
    auto high = _mm512_set1_epi64((value / 10000000000000000ULL) * 10000);
    auto mid = _mm512_set1_epi64((value / 100000000ULL) % 100000000ULL);
    auto low = _mm512_set1_epi64(value % 100000000ULL);

    constexpr uint64_t twoto52 = 1ULL << 52;

    const auto ifma_const = _mm512_setr_epi64(
        twoto52 / 100000000, twoto52 / 10000000, twoto52 / 1000000, twoto52 / 100000,
        twoto52 / 10000, twoto52 / 1000, twoto52 / 100, twoto52 / 10);

    const auto zmmTen = _mm512_set1_epi64(10);
    const auto asciiZero = _mm512_set1_epi64('0');

    auto output = _mm256_setzero_si256();

    auto high_digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, high, ifma_const));
    output = _mm256_mask_expand_epi8(output, 0x0F, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(high_digits)));

    auto mid_digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, mid, ifma_const));
    output = _mm256_mask_expand_epi8(output, 0xFF0, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(mid_digits)));

    auto low_digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, low, ifma_const));
    output = _mm256_mask_expand_epi8(output, 0xFF000, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(low_digits)));

    return output;
}

inline size_t itoa_fast(uint64_t n, char *out)
{
    auto digits = generate_digits(n);

    uint32_t cmp_mask = _mm256_cmpneq_epi8_mask(digits, _mm256_set1_epi8('0'));
    cmp_mask = (cmp_mask & ((1U << 20) - 1)) | (1U << 19);

    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    auto compress_mask = (1U << 21) - _blsi_u32(cmp_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros, compress_mask, digits);
    return 20 - leading_zeros; 
}

inline size_t itoa_fast(int64_t n, char *out)
{
    uint64_t mask = n >> 63;
    uint64_t magnitude = (static_cast<uint64_t>(n) ^ mask) - mask;
    uint64_t is_negative = static_cast<uint64_t>(mask) & 1U;

    auto digits = generate_digits(magnitude);

    uint32_t cmp_mask = _mm256_cmpneq_epi8_mask(digits, _mm256_set1_epi8('0'));
    cmp_mask = (cmp_mask & ((1U << 20) - 1)) | (1U << 19);

    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    uint32_t sign_mask = is_negative << (leading_zeros - 1);
    digits = _mm256_mask_expand_epi8(digits, sign_mask, _mm256_set1_epi8('-'));

    auto compress_mask = (1U << 21) - _blsi_u32(cmp_mask|sign_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros + is_negative, compress_mask, digits);
    return 20 - leading_zeros + is_negative;
}

inline __m256i generate_digits(uint32_t value)
{
    auto mid = _mm512_set1_epi64(value / 100000000U);
    auto low = _mm512_set1_epi64(value % 100000000U);

    constexpr uint64_t twoto52 = 1ULL << 52;

    const auto ifma_const = _mm512_setr_epi64(
        twoto52 / 100000000, twoto52 / 10000000, twoto52 / 1000000, twoto52 / 100000,
        twoto52 / 10000, twoto52 / 1000, twoto52 / 100, twoto52 / 10);

    const auto zmmTen = _mm512_set1_epi64(10);
    const auto asciiZero = _mm512_set1_epi64('0');

    auto output = _mm256_setzero_si256();

    auto mid_digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, mid, ifma_const));
    output = _mm256_mask_expand_epi8(output, 0xFF, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(mid_digits)));

    auto low_digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, low, ifma_const));
    output = _mm256_mask_expand_epi8(output, 0xFF00, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(low_digits)));

    return output;
}


inline size_t itoa_fast(uint32_t n, char *out)
{
    auto digits = generate_digits(n);

    uint32_t cmp_mask = _mm256_cmpneq_epi8_mask(digits, _mm256_set1_epi8('0'));
    cmp_mask = (cmp_mask & ((1U << 16) - 1)) | (1U << 15);

    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    uint32_t compress_mask = (1U << 17) - _blsi_u32(cmp_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros, compress_mask, digits);
    return 16 - leading_zeros;
}

inline size_t itoa_fast(int32_t n, char *out)
{
    int32_t mask = n >> 31;
    uint32_t magnitude = (static_cast<uint32_t>(n) ^ mask) - mask;
    uint32_t is_negative = static_cast<uint32_t>(mask) & 1U;

    auto digits = generate_digits(magnitude);

    uint32_t cmp_mask = _mm256_cmpneq_epi8_mask(digits, _mm256_set1_epi8('0'));
    cmp_mask = (cmp_mask & ((1U << 16) - 1)) | (1U << 15);

    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    uint32_t sign_mask = is_negative << (leading_zeros - 1);
    digits = _mm256_mask_expand_epi8(digits, sign_mask, _mm256_set1_epi8('-'));

    auto compress_mask =  (1U << 17) - _blsi_u32(cmp_mask|sign_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros + is_negative, compress_mask, digits);
    return 16 - leading_zeros + is_negative;
}
