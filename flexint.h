/* flex integer type definitions */

#ifndef FLEXINT_H
#define FLEXINT_H

#ifndef FLEX_NEED_INTEGRAL_TYPE_DEFINITIONS
#include <inttypes.h>
#else
/* Exact integral types.  */

/* Signed.  */

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;

# if __WORDSIZE == 64 || defined __arch64__
typedef long int int64_t;
# else
typedef long long int int64_t;
# endif

/* Unsigned.  */

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

#if __WORDSIZE == 64 || defined __arch64__
typedef unsigned long int uint64_t;
#else
typedef unsigned long long int uint64_t;
#endif
#endif /* FLEX_NEED_INTEGRAL_TYPE_DEFINITIONS */

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


#endif /* ! FLEXINT_H */
