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
#include <linux/string.h>
#include "common.h"
#include "module.h"
#include "fpga_dot_font.h"

#define DEVICE_SET_MAJOR 242
#define DEVICE_SET_NAME "dev_driver"

// 디바이스 물리 메모리 주소
#define IOM_FND_ADDRESS 0x08000004
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FPGA_DOT_ADDRESS 0x08000210

// 디바이스 가상 메모리 주소
static unsigned char *fnd_addr, *text_lcd_addr, *led_addr, *dot_addr;
settings setting;

char *student_num = "20141494";
char *student_name = "KangDongWook";

// 파일 오퍼레이션 설정
struct file_operations device_set_file_fops =
        {
                .unlocked_ioctl = device_set_file_ioctl,
        };


int device_set_file_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

    // printk("ioctl %u %u %u %u\n",_IOC_TYPE(cmd),_IOC_NR(cmd), _IOC_DIR(cmd), _IOC_SIZE(cmd));

    switch (cmd)
    {
        case CMD_SETTING:
            // 설정 커맨드 일 경우 유저 스페이스에서 커널 스페이스로 설정 복사
            if (copy_from_user(&setting,(void __user*)arg, sizeof(setting))) {
                return -EFAULT;
            }
            break;
        case CMD_EXECUTE:
            // 타이머 실행
            execute_timer();

            break;

    }
}

void delay()
{
    // 현재시각 + interval * 0.1 초 후에 awake
    u64 endtime = get_jiffies_64() + setting.interval * HZ / 10;
    while (get_jiffies_64() < endtime);
}

void execute_timer()
{
    int init = setting.init;
    int displaying_value = 0;
    int displaying_index = 4;
    int cycle_count = 0;
    int count;
    char fnd_val[4];

    // init 변수에서 위치와 숫자 추출
    while (displaying_value == 0 && displaying_index > 0)
    {
        displaying_value = init % 10;
        init /= 10;
        displaying_index--;
    }


    for (count = 0; count < setting.count; count++)
    {
        // fnd 에 표시문자 설정
        memset(fnd_val, 0, 4);
        fnd_val[displaying_index] = displaying_value;
        set_fnd(fnd_val);

        set_led(1 << (8 - displaying_value));

        set_dot(fpga_number[displaying_value]);

        set_text_lcd(cycle_count);

        // interval 만큼 대기
        delay();
        cycle_count++;

        // 8개의 숫자가 한번씩 표시되었다면 fnd 표시위치 변경
        if (cycle_count % 8 == 0)
        {
            displaying_index = (displaying_index + 1) % 4;
        }
        // 표기 숫자 변경
        displaying_value = (displaying_value) % 8 + 1;

    }

    // 타이머 종료후 표기 초기화
    memset(fnd_val, 0, 4);
    set_fnd(fnd_val);
    set_led(0);
    set_dot(fpga_set_blank);
    set_text_lcd_blank();
    
}


int __init device_set_init(void)
{
    int result;

    // character device 등록
    result = register_chrdev(DEVICE_SET_MAJOR, DEVICE_SET_NAME, &device_set_file_fops);
    if (result < 0)
    {
        printk(KERN_WARNING
        "Can't get any major\n");
        return result;
    }

    // 디바이스 제어를 위한 물리 메모리 주소를 가상 메모리 주소로 변환
    fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
    text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
    led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
    dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);

    printk("init module, major number : %d\n", DEVICE_SET_MAJOR);

    return 0;
}

// text_lcd에서 좌우로 움직이는 글씨를 현재 싸이클에 맞는 위치로 복사한다.
void set_moving_lcd_text(char *dest, char *text, int cycle_num)
{
    char str[16] = {0,};
    int max_offset = 16 - strlen(text);
    // 최대 4칸을 움직일 수 있는 글씨의 경우, 01234321 순서대로의 칸수만큼 offset을 가진다.
    // 현재 사이클이 01234321중 어떤 offset값의 인덱스를 가지는지 알기위해 0~7 사이의 inner_cycle 값을 가정한다.
    int inner_cycle = cycle_num % (max_offset * 2);
    int offset, i;

    // 현재 offset 값을 계산한다.
    if (inner_cycle >= max_offset)
    {
        offset = max_offset * 2 - inner_cycle;
    }
    else
    {
        offset = inner_cycle;
    }

    //복사
    memset(dest, ' ', 16);
    for (i = 0; i < strlen(text); i++)
    {
        dest[offset + i] = text[i];
    }

}

void set_text_lcd(int cycle_count)
{
    char str[33] = {0,};
    int i;

    // 첫째줄 표시
    set_moving_lcd_text(str, student_num, cycle_count);
    // 둘째줄 표시
    set_moving_lcd_text(str + 16, student_name, cycle_count);

    unsigned short int value_short = 0;


    // text lcd 표기
    for (i = 0; i < 32; i = i + 2)
    {
        value_short = (str[i] & 0xFF) << 8 | str[i + 1] & 0xFF;
        outw(value_short, (unsigned int) text_lcd_addr + i);
    }

}

// lcd 초기화
void set_text_lcd_blank()
{
    char blank = ' ';
    unsigned short int value_short = 0;
    int i;

    for (i = 0; i < 32; i = i + 2)
    {
        value_short = blank << 8 | blank;
        outw(value_short, (unsigned int) text_lcd_addr + i);
    }
}

void set_fnd(unsigned char value[4])
{
    unsigned short int value_short = 0;

    value_short = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
    outw(value_short, (unsigned int) fnd_addr);
}

void set_led(unsigned char value)
{
    outw(value, (unsigned int) led_addr);
}


void set_dot(unsigned char value[10])
{
    int i;
    unsigned short int value_short = 0;

    for (i = 0; i < 10; i++)
    {
        value_short = value[i] & 0x7F;
        outw(value_short, (unsigned int) dot_addr + i * 2);
    }
}

void __exit device_set_exit(void)
{
    iounmap(fnd_addr);
    iounmap(led_addr);
    iounmap(text_lcd_addr);
    iounmap(dot_addr);
    unregister_chrdev(DEVICE_SET_MAJOR, DEVICE_SET_NAME);
}

module_init(device_set_init);
module_exit(device_set_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("20141494");
