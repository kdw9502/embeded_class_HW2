#pragma once
typedef struct _settings
{
	char interval;
	char count;
	char init;
}settings;

#define IOCTL_MAGIC         'T'
#define CMD_SETTING _IOWR(IOCTL_MAGIC,0, settings)
#define CMD_EXCUTE _IO(IOCTL_MAGIC, 1)

