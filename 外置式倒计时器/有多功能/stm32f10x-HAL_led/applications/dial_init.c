
#include <rtthread.h>
#include <rtdevice.h>
#include <stm32f1xx_hal.h>
#include "config.h"

config_t config_init;


static  rt_uint16_t read_hc165(void)
{
	rt_uint16_t boma_data=0;
	rt_uint8_t  loop_i=0;
	rt_pin_write(PL,0);
	HAL_Delay(10);
	rt_pin_write(PL,1);
	HAL_Delay(10);
	boma_data=rt_pin_read(DS);
	for(loop_i=0;loop_i<15;loop_i++)
	{
		rt_pin_write(CP,0);
		boma_data=(boma_data<<1);
		HAL_Delay(1);
		rt_pin_write(CP,1);
		HAL_Delay(1);
		boma_data|=rt_pin_read(DS);
		
	}
	HAL_Delay(5);
	return boma_data;
}

void rt_hw_dial_detection(void)
{

		rt_uint16_t data_a=0,data_b=0;
		rt_uint8_t data_c=0,data_d=0,data_e=0;
		HAL_Delay(10);
		do{
			data_a=read_hc165();
			data_b=read_hc165();
		}
		while(data_a!=data_b);

		if(data_b&0x0004)//通讯型倒计时 学习型倒计时选择
		{
			config_init.study_or_comm =0 ;//通信型
			if(data_b&0x0100)
			config_init.baud_rate=9600;//波特率
			else
			config_init.baud_rate=4800;	
			if(data_b &0x0002)//协议选择
			{
				 data_c = (data_b&0x0e00)>>8;
				 data_d = (data_b&0xf000)>>8;
					
				 data_c=(data_c<<4)|(data_c>>4);  
				 data_c=((data_c<<2)&0xcc)|((data_c>>2)&0x33);  
				 data_c=((data_c<<1)&0xaa)|((data_c>>1)&0x55);  
				 data_c= data_c>>4;
					
				 data_d=(data_d<<4)|(data_d>>4);  
				 data_d=((data_d<<2)&0xcc)|((data_d>>2)&0x33);  
				 data_d=((data_d<<1)&0xaa)|((data_d>>1)&0x55);
				
				config_init.protocol=1;//格林威协议
				config_init.countdown_direction=0xb0+data_c;
				config_init.singal_direction=0xc0+data_d;
			}
			else
			{
				
				data_e = (data_b>>8);
				data_e=(data_e<<4)|(data_e>>4);  
				data_e=((data_e<<2)&0xcc)|((data_e>>2)&0x33);  
			  data_e=((data_e<<1)&0xaa)|((data_e>>1)&0x55); 
				data_e=data_e&0x0f ;				
				config_init.protocol=0;//国标协议
				config_init.countdown_channel=data_e;
			}
		}
		else
		{
			config_init.study_or_comm =1 ;//学习型
		}

		if(data_b&0x0001)
			config_init.r_led=1;
		else
			config_init.r_led=0;//只显示红
				
		if(data_b&0x0008)//机动学习还是行人学习
			config_init.motor_or_people = 1;//机动
			else
		  config_init.motor_or_people = 0;//人行
				
}

void print_info(void)
	{	
			rt_kprintf("the baud_rate is %d \r\n",config_init.baud_rate);	
			if(0==config_init.study_or_comm )
				{
					rt_kprintf("the study_or_comm is comm\r\n");
					
					if(1==config_init.protocol)
					{
						rt_kprintf("the protocol is ge_lin_wei \r\n");
							switch(config_init.countdown_direction)
					{
						case 0xb0:rt_kprintf("the countdonw_direction is east \r\n");break;
						case 0xb1:rt_kprintf("the countdonw_direction is west \r\n");break;
						case 0xb2:rt_kprintf("the countdonw_direction is south \r\n");break;
						case 0xb3:rt_kprintf("the countdonw_direction is north \r\n");break;
						case 0xb4:rt_kprintf("the countdonw_direction is aux_1 \r\n");break;
						case 0xb5:rt_kprintf("the countdonw_direction is aux_2 \r\n");break;
						case 0xb6:rt_kprintf("the countdonw_direction is aux_3 \r\n");break;
						case 0xb7:rt_kprintf("the countdonw_direction is aux_4 \r\n");break;
						default:rt_kprintf("the countdonw_direction is default %d \r\n",config_init.countdown_direction);
					}
				
				switch(config_init.singal_direction)
				{
					case 0xc0:rt_kprintf("the singal_direction is full screen lights \r\n");break;
					case 0xc1:rt_kprintf("the singal_direction is left direction \r\n");break;
					case 0xc2:rt_kprintf("the singal_direction is straight direction \r\n");break;
					case 0xc3:rt_kprintf("the singal_direction is right direction \r\n");break;
					case 0xc4:rt_kprintf("the singal_direction is bus rode \r\n");break;
					case 0xc5:rt_kprintf("the singal_direction is person first pass street 1 \r\n");break;
					case 0xc6:rt_kprintf("the singal_direction is person first pass street 2 \r\n");break;
					case 0xc7:rt_kprintf("the singal_direction is person second pass street 1 \r\n");break;
					case 0xc8:rt_kprintf("the singal_direction is person second pass street 2 \r\n");break;
					case 0xc9:rt_kprintf("the singal_direction is no motor way \r\n");break;
					case 0xca:rt_kprintf("the singal_direction is led screen \r\n");break;
					case 0xcb:rt_kprintf("the singal_direction is l_and_a led \r\n");break;
					case 0xcc:rt_kprintf("the singal_direction is a_and_r led \r\n");break;
					case 0xcd:rt_kprintf("the singal_direction is l_and_r led \r\n");break;
					default:  rt_kprintf  ("the singal_direction is default %d \r\n",config_init.singal_direction);
				}	
					}
					else if(0==config_init.protocol)
					{
						rt_kprintf("the protocol is national_standard \r\n");
						rt_kprintf("the countdown_channel is %d \r\n",config_init.countdown_channel);
					}
				}
			else if(1==config_init.study_or_comm)
				{	
					rt_kprintf("the study_or_comm is study\r\n");
					
					if(0==config_init.motor_or_people)
						rt_kprintf("the motor_or_people is people\r\n");
					else if(1==config_init.motor_or_people)
						rt_kprintf("the motor_or_people is motor\r\n");
					
				}
		
				if(0==config_init.r_led)
					rt_kprintf("the r_led is only red \r\n");
				else if(1==config_init.r_led)
					rt_kprintf("the r_led is normal  \r\n");
	}

int rt_hw_dial_init(void)
{

	  rt_pin_mode(DS, PIN_MODE_INPUT);
		rt_pin_mode(CP, PIN_MODE_OUTPUT);
		rt_pin_mode(PL, PIN_MODE_OUTPUT);
		
		rt_pin_mode(LEFT_R,   PIN_MODE_INPUT);  
		rt_pin_mode(LEFT_Y,   PIN_MODE_INPUT);  
		rt_pin_mode(LEFT_G,   PIN_MODE_INPUT);  
		rt_pin_mode(AHEAD_R,   PIN_MODE_INPUT);  
		rt_pin_mode(AHEAD_Y,   PIN_MODE_INPUT);  
		rt_pin_mode(AHEAD_G,   PIN_MODE_INPUT);  
		rt_pin_mode(RIGHT_R,   PIN_MODE_INPUT);  
		rt_pin_mode(RIGHT_Y,   PIN_MODE_INPUT);  
		rt_pin_mode(RIGHT_G,   PIN_MODE_INPUT);  
		
		rt_hw_dial_detection();
	//-------------------------------------------------------------------TEST------------------------------------------------------------------START
//		config_init.baud_rate=4800;//最大支持9600
//		config_init.countdown_channel=0;//国标协议对应的通道
//		config_init.countdown_direction=0xb6;//格林威协议对应的倒计时方向
//		config_init.motor_or_people=1;//0---人行   1--机动
//		config_init.protocol=0;//0---国标  1----格林威
//		config_init.r_led=1;//1--都显示  0---只显示红
//		config_init.singal_direction=0xc7;//格林倒计时通道
//		config_init.study_or_comm=0;//0---通讯型  1---学习型
	//-------------------------------------------------------------------TEST------------------------------------------------------------------END	
		print_info();
		//rt_hw_uart_init();
		return 0 ;
}

		INIT_ENV_EXPORT(rt_hw_dial_init);
		MSH_CMD_EXPORT(rt_hw_dial_init, dial sample);
