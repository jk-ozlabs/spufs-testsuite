#ifndef _PATTERN_H
#define _PATTERN_H

#include <stdint.h>

void load_pattern(uint8_t *buf, int len);
int check_pattern(uint8_t *buf, int len);

#endif /* _PATTERN_H */
