#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <asm/ioctl.h>
#include "common.h"
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char* argv[])
{
    // arguments 개수 체크
    if (argc < 4)
    {
        printf("Usage: %s TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]", argv[0]);
        return -1;
    }

	settings setting;
	setting.interval = atoi(argv[1]);
	setting.count = atoi(argv[2]);
	setting.init = atoi(argv[3]);

	

	// arguments 범위 체크
	if (setting.interval > 100 || setting.interval <= 0 ||
		setting.count > 100 || setting.count <= 0 ||
		setting.init > 8000 || setting.init <= 0)
	{
		printf("Usage: %s TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]", argv[0]);
		return -1;
	}
	
	int fd = open("/dev/dev_driver", O_RDWR);
	if (fd < 0) {
		perror("/dev/dev_driver error");
		return -1;
	}

	// 설정
	ioctl(fd, CMD_SETTING, &setting);

	// 디바이스 구동
	ioctl(fd, CMD_EXECUTE);


	close(fd);

	
}
