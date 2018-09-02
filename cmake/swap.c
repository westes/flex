#include <stdint.h>

uint32_t htonl(uint32_t x)
{
#if HOST_IS_LITTLE_ENDIAN
    uint8_t *s = (uint8_t *)&x;
    return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
#else
    return x;
#endif
}
uint16_t htons(uint16_t x)
{
#if HOST_IS_LITTLE_ENDIAN
    uint8_t *s = (uint8_t *)&x;
    return (uint16_t)(s[0] << 8 | s[1]);
#else
    return x;
#endif
}
