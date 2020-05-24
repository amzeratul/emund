#pragma once

#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif