#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#include "common.h"
#include "module.h"
#include <math.h>
#include <string.h>
#include "fpga_dot_font.h"

#define DEVICE_SET_MAJOR 242		
#define DEVICE_SET_NAME "device_set"		

#define IOM_FND_ADDRESS 0x08000004 
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FPGA_DOT_ADDRESS 0x08000210

static unsigned char *fnd_addr, *text_lcd_addr, *led_addr, *dot_addr;
settings setting;

char* student_num = "20141494";
char* student_name = "KangDongWook";


struct file_operations device_set_file_fops =
{
	.unlocked_ioctl = device_set_file_ioctl,
};


int device_set_file_ioctl(struct inode* minode, struct file* filp, unsigned int cmd, unsigned long arg)
{
	
	switch (cmd)
	{
	case CMD_SETTING:
		if (copy_from_user(&setting, (void __user*)arg, sizeof(setting))) {
			return -EFAULT;
		}
		break;
	case CMD_EXCUTE:
		excute_timer();
		
		break;

	}
}

void delay()
{

	u64 endtime = get_jiffies() + setting.interval * HZ / 1000;
	while (get_jiffies() < endtime);
}

void excute_timer()
{
	int init = setting.init;
	int displaying_value = 0;
	int displaying_index = 3;
	int cycle_count = 0;
	int count;


	while (displaying_value != 0)
	{
		displaying_value = init % 10;
		init /= 10;
		displaying_index--;
	}

	for (count = 0; count < setting.count; count++)
	{
		char fnd_val[4] = { 0, };
		fnd_val[displaying_index] = displaying_value;
		set_fnd(fnd_val);
		set_led(1<<(7-displaying_value));
		set_dot(fpga_number[displaying_value]);
		set_text_lcd(cycle_count);
		delay();
		cycle_count++;
		if (cycle_count % 8 == 0)
		{
			displaying_index = (displaying_index + 3) % 4;
		}
		displaying_value = (displaying_value - 1) % 8 + 1;
		
	}


	
}



int __init device_set_init(void)
{
	int result;

	result = register_chrdev(DEVICE_SET_MAJOR, DEVICE_SET_NAME, &device_set_file_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
	led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);

	printk("init module, major number : %d\n", DEVICE_SET_MAJOR);

	return 0;
}

// text_lcd���� �¿�� �����̴� �۾��� ���� ����Ŭ�� �´� ��ġ�� �����Ѵ�.
void set_moving_lcd_text(char* dest, char* text, int cycle_num)
{
	char str[16] = { 0, };
	int max_offset = 16 - strlen(text);
	// �ִ� 4ĭ�� ������ �� �ִ� �۾��� ���, 01234321 ��������� ĭ����ŭ offset�� ������.
	// ���� ����Ŭ�� 01234321�� � offset���� �ε����� �������� �˱����� 0~7 ������ inner_cycle ���� �����Ѵ�.
	int inner_cycle = cycle_num % (max_offset * 2);
	int offset,i;

	// ���� offset ���� ����Ѵ�.
	if (inner_cycle >= max_offset)
	{
		offset = max_offset * 2 - inner_cycle;
	}
	else 
	{
		offset = inner_cycle;
	}

	//����
	memset(dest,' ', 16);

	for (int i = 0; i < strlen(text); i++)
	{
		dest[offset + i] = text[i];
	}

}

void set_text_lcd(int cycle_count)
{
	char str[33] = {0,};
	int i;
	
	set_moving_lcd_text(str, student_name, cycle_count);
	set_moving_lcd_text(str+16, student_name, cycle_count);
	
	unsigned short int value_short = 0;


	for (i = 0; i < 32; i=i+2)
	{
		value_short = (value[i] & 0xFF) << 8 | value[i + 1] & 0xFF;
		outw(value_short, (unsigned int)text_lcd_addr + i);
	}

}

void set_fnd(unsigned char value[4])
{
	unsigned short int value_short = 0;

	value_short = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
	outw(value_short, (unsigned int)fnd_addr);
}

void set_led(unsigned char value)
{
	outw(value, (unsigned int)led_addr);
}


void set_dot(unsigned char value[10])
{
	int i;
	unsigned short int value_short = 0;

	for (i = 0; i < length; i++)
	{
		value_short = value[i] & 0x7F;
		outw(value_short, (unsigned int)dot_addr + i * 2);
	}
}

void __exit device_set_exit(void) 
{
	iounmap(fnd_addr);
	iounmap(led_addr);
	iounmap(text_lcd_addr);
	iounmap(dot_addr);
	unregister_chrdev(DEVICE_SET_MAJOR, IOM_FND_NAME);
}

module_init(device_set_init);
module_exit(device_set_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("20141494");
