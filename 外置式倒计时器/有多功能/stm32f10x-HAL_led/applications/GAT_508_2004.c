#include <rtthread.h>
#include <rtdevice.h>
#include "config.h"
void gat_2004_parse(config_t config)
{
	rt_uint8_t rcv_buf[128]={0};
	rt_uint8_t buff_len = 0;
	rt_uint8_t mm = 0;
	rt_uint8_t crc = 0;
	rt_uint8_t loop_i = 0;
	buff_len = config.rx_counter;
	
	rt_memcpy(rcv_buf,config.rx_buffer,config.rx_counter);
	while(1)
	{
		if(config.countdown_channel == ((rcv_buf[mm+1]&0x1c)>>2))
		{
			for(loop_i=0;loop_i<3;loop_i++)
			crc ^=rcv_buf[mm+1+loop_i];
			
			crc &=0x7f;
			if(crc == rcv_buf[mm+4])
			{
				countdown.cur_tmp=((rcv_buf[mm+2]&0xf0)>>4)*1000+(((rcv_buf[mm+2]&0x0f))*100)+ ((rcv_buf[mm+3]&0xf0)>>4)*10+(rcv_buf[mm+3]&0x0f);
				if(0x01 == (rcv_buf[mm+1]&0x03))
				{
				
						countdown.flag_g = 1;
						countdown.mode_g = 1;
						countdown.cur_num_g=countdown.cur_tmp;					
						countdown.cur_mode=GREEN_MODE;
						countdown.test=4;
				}
				else if(0x03 == (rcv_buf[mm+1]&0x03))
					{
						countdown.flag_r = 1;
						countdown.mode_r = 1;
						countdown.cur_num_r=countdown.cur_tmp;
						countdown.cur_mode=RED_MODE;
						countdown.test=3;
					}
				else if(0x02 == (rcv_buf[mm+1]&0x03))
				{
						countdown.flag_y = 1;
						countdown.mode_y = 1;
						countdown.cur_num_y=countdown.cur_tmp;
						countdown.cur_mode=YELLOW_MODE;
						countdown.test=5;
				}
				else if(0x00 == (rcv_buf[mm+1]&0x03))
						countdown.cur_mode=BLACK_MODE;
				
				if(0x00 ==((rcv_buf[mm+1]&0x20)>>5))
					countdown.type= 0;
				else if(0x01 ==((rcv_buf[mm+1]&0x20)>>5))
				{
					countdown.type= 1;
				}
				
				break;
			}
			else
			{
				break;
			}
		}
		else
		{
			mm=mm+5;
			if(mm > buff_len)
				break;
		}
	}
	
}
