#include <rtthread.h>
#include <rtdevice.h>
#include "config.h"
void gat_2014_parse(config_t config)
{
	rt_uint8_t buf_len=0;
	rt_uint8_t rx_buf[128]={0};
	rt_uint8_t loop_i=0;
	rt_uint8_t mm =0;
	rt_uint8_t crc = 0;
	buf_len=config.rx_counter;
	rt_memcpy(rx_buf,config.rx_buffer,config.rx_counter);
	rt_memcpy(config.rx_buffer,0,config.rx_counter);
	
	if((buf_len-4) == (rx_buf[2]*2))//验证长度
	{
		for(loop_i=0;loop_i<(buf_len-3);loop_i++)
		crc ^= rx_buf[loop_i+2];
		
		if(crc == rx_buf[buf_len -1])//校验
		{
			while(1)
			{
				if(config.countdown_channel == (rx_buf[3+mm]>>3))
					{
						countdown.cur_tmp=rx_buf[4+mm];	
						if(0x01==(rx_buf[3+mm]&0x03))
						{
							countdown.cur_num_g=countdown.cur_tmp;
							countdown.mode_g = 1;
							countdown.flag_g = 1;
						}
						else if(0x03==(rx_buf[3+mm]&0x03))
						{
							countdown.cur_num_r=countdown.cur_tmp;
							countdown.mode_r = 1;
							countdown.flag_r = 1;
						}
						else if(0x02==(rx_buf[3+mm]&0x03))
						{
							countdown.cur_num_y=countdown.cur_tmp;
							countdown.mode_y = 1;
							countdown.flag_y = 1;
						}
						else if(0x00==(rx_buf[3+mm]&0x03))
							countdown.cur_mode=BLACK_MODE;
						
						if(0x00 == ((rx_buf[3+mm]&0x4)>>2))
							countdown.blind = 0;
						else if(0x01 == ((rx_buf[3+mm]&0x4)>>2))
							countdown.blind = 1;
						
						//countdown.cur_num=rx_buf[4+mm];	
						countdown.type = 0;//2014 协议无自动倒计时
						
						break;
					}
				else
					{
						mm=mm+2;
						if(mm > (buf_len-4))
							break;
					}
			}
		
		}
	}
	
	
}
