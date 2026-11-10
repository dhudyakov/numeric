High-performance AVX-512 integer-to-string (itoa) conversion targeting x86_64 architectures.

Based on the paper:
"Converting an Integer to a Decimal String in Under Two Nanoseconds" by Jaël Champagne Gareau and Daniel Lemire (Software: Practice and Experience, 2026).
https://arxiv.org/abs/2604.26019

Branchless and pipelined architecture. Digits are processed in 3 independent blocks. While each block contains a pair of structurally dependent AVX-512 IFMA instructions (_mm512_madd52lo_epu64 and _mm512_madd52hi_epu64) carrying an 8-cycle dependency chain, the blocks are decoupled and interleaved. Block 2 initiates at cycle 1 and Block 3 initiates at cycle 2 to maximize pipeline port occupancy. BMI instructions then handle string boundaries and null termination.
