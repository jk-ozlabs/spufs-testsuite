#include <unistd.h>
#include <sys/types.h>

int main(void)
{
	return getuid() != 0;
}
