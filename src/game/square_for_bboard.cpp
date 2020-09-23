

#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace siegbert {

uint8_t square_for_bboard(uint64_t l) {
#if defined(__GNUC__) || defined(__clang__)
  int offset = sizeof(uint64_t) * 8 - 1 - __builtin_clzl(l);
  return 16 * (offset / 8) + (offset % 8);
#elif defined(_MSC_VER)
#pragma intrinsic(_BitScanReverse64)
  unsigned long offset;
  _BitScanReverse64(&offset, l);
  return 16 * (offset / 8) + (offset % 8);
#else
  for (int row = 0; row < 8; row += 1) {
    for (int col = 0; col < 8; col += 1) {
      if (l & (1 << (row * 8 + col))) {
        return 16 * row + col;
      }
    }
  }
  return 0x88;
#endif
}

} // namespace siegbert
