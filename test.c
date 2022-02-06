#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	int fd;
	char buf[32];

	fd = open("/dev/my_uart", O_RDONLY);
	printf("fd: %d\n", fd);
	if(fd < 0) {
		return -1;
	}

	printf("n: %d\n", read(fd, buf, 5));
	close(fd);
	return 0;
}	
