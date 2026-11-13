#pragma once
#include <immintrin.h>
#include <x86intrin.h>
#include <iostream>
#include <iomanip>
#include <stdint.h>

inline __m256i convert8(uint64_t value,__mmask32 mask)
{
    constexpr uint64_t twoto52 = 1ULL << 52;

    const auto ifma_const = _mm512_setr_epi64(
        twoto52 / 100000000, twoto52 / 10000000, twoto52 / 1000000, twoto52 / 100000,
        twoto52 / 10000, twoto52 / 1000, twoto52 / 100, twoto52 / 10);

    const auto zmmTen = _mm512_set1_epi64(10);
    const auto asciiZero = _mm512_set1_epi64('0');

    auto digits = _mm512_madd52hi_epu64(asciiZero, zmmTen, _mm512_madd52lo_epu64(ifma_const, _mm512_set1_epi64(value), ifma_const));
    return _mm256_maskz_expand_epi8(mask, _mm256_castsi128_si256(_mm512_cvtepi64_epi8(digits)));
}

inline __m256i generate_digits(uint64_t value)
{
    auto high = convert8((value / 10000000000000000ULL) * 10000, 0x0F);
    auto mid = convert8((value / 100000000ULL) % 100000000ULL, 0xFF0);
    auto low = convert8(value % 100000000ULL, 0xFF000);

    return  _mm256_or_si256(_mm256_or_si256(high, mid), low);
}

inline size_t itoa_fast(uint64_t n, char *out)
{
    auto digits = generate_digits(n);

    uint32_t cmp_mask = _mm256_mask_cmpneq_epi8_mask(0xFFFFF, digits, _mm256_maskz_set1_epi8(0x7FFFF, '0'));
    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    auto compress_mask = (1U << 21) - _blsi_u32(cmp_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros, compress_mask, digits);

    return 20 - leading_zeros; 
}

inline size_t itoa_fast(int64_t n, char *out)
{
    uint64_t u = static_cast<uint64_t>(n);
    uint64_t magnitude = (n < 0) ? -u : u;
    uint32_t is_negative = static_cast<uint32_t>(u >> 63);

    auto digits = generate_digits(magnitude);

    uint32_t cmp_mask = _mm256_mask_cmpneq_epi8_mask(0xFFFFF, digits, _mm256_maskz_set1_epi8(0x7FFFF, '0'));
    uint32_t leading_zeros = __builtin_ctz(cmp_mask);

    uint32_t first_bit = _blsi_u32(cmp_mask);
    uint32_t start_bit = first_bit >> is_negative;
    digits = _mm256_mask_mov_epi8(digits, start_bit & ~first_bit, _mm256_set1_epi8('-'));
    
    auto compress_mask   = (1U << 21) - start_bit;
    _mm256_mask_storeu_epi8(out - leading_zeros + is_negative, compress_mask, digits);
    return 20 - leading_zeros + is_negative;
}

inline __m256i generate_digits(uint32_t value)
{
    auto mid = convert8(value / 100000000U, 0xFF);
    auto low = convert8(value % 100000000U, 0xFF00);

    return  _mm256_or_si256(mid, low);
}


inline size_t itoa_fast(uint32_t n, char *out)
{
    auto digits = generate_digits(n);

    uint32_t cmp_mask = _mm256_mask_cmpneq_epi8_mask(0xFFFF, digits, _mm256_maskz_set1_epi8(0x7FFF, '0'));
    uint32_t leading_zeros = __builtin_ctz(cmp_mask);
    uint32_t compress_mask = (1U << 17) - _blsi_u32(cmp_mask);
    _mm256_mask_storeu_epi8(out - leading_zeros, compress_mask, digits);
    return 16 - leading_zeros;
}

inline size_t itoa_fast(int32_t n, char *out)
{
    uint32_t u = static_cast<uint32_t>(n);
    uint32_t magnitude = (n < 0) ? -u : u;
    uint32_t is_negative = u >> 31;

    auto digits = generate_digits(magnitude);
    uint32_t cmp_mask = _mm256_mask_cmpneq_epi8_mask(0xFFFF, digits, _mm256_maskz_set1_epi8(0x7FFF, '0'));
    uint32_t leading_zeros = __builtin_ctz(cmp_mask);

    uint32_t first_bit = _blsi_u32(cmp_mask);
    uint32_t start_bit = first_bit >> is_negative;
    digits = _mm256_mask_mov_epi8(digits, start_bit & ~first_bit, _mm256_set1_epi8('-'));

    uint32_t compress_mask   = (1U << 17) - start_bit;
    _mm256_mask_storeu_epi8(out - leading_zeros + is_negative, compress_mask, digits);
    return 16 - leading_zeros + is_negative;
}
