#pragma once

void set_moving_lcd_text(char* dest, char* text, int cycle_num);

void set_text_lcd(int cycle_count);

void set_fnd(unsigned char value[4]);

void set_led(unsigned char value);

void set_dot(unsigned char value[10]);
int device_set_file_ioctl(struct file* filp, unsigned int cmd, unsigned long arg);
void excute_timer();
