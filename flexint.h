/* flex integer type definitions */

#ifndef FLEXINT_H
#define FLEXINT_H

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901
#include <stdint.h>
#else
/* Exact integral types.  */

/* Signed.  */

#define int8_t signed char
#define int16_t short int
#define int32_t int

# if __WORDSIZE == 64 || defined __arch64__
#define int64_t long int
# else
#define int64_t long long int
# endif

/* Unsigned.  */

#define uint8_t unsigned char
#define uint16_t unsigned short int
#define uint32_t unsigned int

#if __WORDSIZE == 64 || defined __arch64__
#define uint64_t unsigned long int
#else
#define uint64_t unsigned long long int
#endif

/* Limits of integral types. */
#ifndef INT8_MIN
#define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX              (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX             (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif

#endif /* not C99 system */

#endif /* ! FLEXINT_H */
