// 모듈과 앱 양쪽에서 쓰일 셋팅 구조체
typedef struct _settings
{
	char interval;
	char count;
	int init;
}settings;

#define IOCTL_MAGIC         'T'
#define CMD_SETTING _IOWR(IOCTL_MAGIC,0, settings)
#define CMD_EXECUTE _IO(IOCTL_MAGIC, 1)

