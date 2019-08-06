#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <rtthread.h>
#include <rtdevice.h>

#define true 1
#define false 0

typedef struct count_down
{
	rt_uint8_t cur_mode;
	rt_uint8_t blind;
	rt_uint8_t type;
	rt_uint8_t sync;//倒计时同步 防止倒计时数据生成时 不到一秒就递减
	rt_uint8_t flag_r;
	rt_uint8_t flag_y;
	rt_uint8_t flag_g;
	rt_uint8_t mode_r;
	rt_uint8_t mode_y;
	rt_uint8_t mode_g;
	rt_uint16_t cur_num;
	rt_uint16_t cur_tmp;
	rt_uint16_t cur_num_r;
	rt_uint16_t cur_num_y;
	rt_uint16_t cur_num_g;
	rt_uint8_t test;
	
}countdown_t;

//该结构体都是通过拨码开关来选择配置 
typedef struct config{
 rt_uint16_t baud_rate;//波特率选择 最大支持 9600  
 rt_uint8_t  countdown_direction;//倒计时方向 只对应格林威协议
 rt_uint8_t  singal_direction;//信号灯方向 只对应格林威协议
 rt_uint8_t  protocol;//协议选择
 rt_uint8_t  r_led;//只显示红色
 rt_uint8_t  countdown_channel;//倒计时方向 只对应国标2004 和2014  此时倒计时方向和信号方向 合起来为倒计时通道 
 rt_uint8_t  motor_or_people;//学习型的倒计时用作 机动学习还是行人学习
 rt_uint8_t  study_or_comm;//倒计时用作通讯型还是学习型
 rt_uint8_t  rx_buffer[512];  //由256增大至512
 rt_uint16_t  rx_counter;		//由8位改为16位	
}config_t;
//格林解析
typedef struct gelinwei{
	rt_uint8_t dir;//方向  
	rt_uint8_t cmd;//命令
	rt_uint8_t countdown_type;//倒计时类型  自动倒计时还是一秒有信号机发信号刷新  详见格林威协议
	rt_uint8_t reflash_flag;//刷新码
	rt_uint8_t addr;//地址
	rt_uint8_t color;//颜色
	rt_uint8_t data_h;//倒计时高位
	rt_uint8_t data_l;//倒计时低位
	rt_uint8_t org_data[128];//原信息码

}gelinwei_info_t;

extern countdown_t countdown;
extern config_t config_init;
extern gelinwei_info_t gelinwei_info;
extern struct  rt_event event_rcv;

#define PL 25  //singal_direction bit6 -- PC5
#define DS 24  //singal_direction bit7 -- PC4
#define CP 23  //singal_direction bit8 -- PA7


#define LEFT_R   rt_pin_read(8)//PC0--8
#define LEFT_Y   rt_pin_read(9)//PC1--9 
#define LEFT_G   rt_pin_read(10)//PC2--10

#define AHEAD_R  rt_pin_read(11)//PC3--11 
#define AHEAD_Y  rt_pin_read(14)//PA0--14
#define AHEAD_G  rt_pin_read(15)//PA1--15

#define RIGHT_R rt_pin_read(20)//PA4--20 
#define RIGHT_Y rt_pin_read(21)//PA5--21
#define RIGHT_G rt_pin_read(22)//PA6--22


#define RD1_PIN   53	 //PC12--53
#define RD2_PIN   50	 //PA15--50	
#define GD1_PIN   52	 //PC11--52
#define GD2_PIN   41	 //PA8 --41
#define BD1_PIN   51	 //PC10--51
#define BD2_PIN   40	 //PC9 --40 	
#define CLK_PIN   57   //PB5--57
#define LAT_PIN   56	 //PB4--56
#define OE_PIN    54   //PD2--54
#define RED_MODE     1
#define GREEN_MODE   2
#define YELLOW_MODE  3
#define BLACK_MODE   4
#define H_MODE 5

#define IIC_SDA //PB7--59
#define IIC_CLK //PB6--58

#define BAUD_RATE  config_init.baud_rate
#define COUNTDOWN_DIRECTION  config_init.countdown_direction
#define SINGAL_DIRECTION  config_init.singal_direction
#define PROTOCOL   config_init.protocol
#define R_LED  config_init.r_led
#define COUNTDOWN_CHANNEL  config_init.countdown_channel
#define STUDY_OR_COMM config_init.study_or_comm
#define MOTRO_OR_PEOPLE config_init.motor_or_people


#define DEBUG_PRINTF 0  //1--printf debug info   0---no printf

void gat_2014_parse(config_t config);
void gat_2004_parse(config_t config);
void ge_lin_wei_parse(config_t config);
void rt_hw_uart_init(void);
#endif
