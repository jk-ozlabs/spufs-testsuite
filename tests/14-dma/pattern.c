
#include <stdint.h>

#include "pattern.h"

void load_pattern(uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		buf[i] = i & 0xff;
}

int check_pattern(uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (buf[i] != (i & 0xff))
			return -1;

	return 0;
}
