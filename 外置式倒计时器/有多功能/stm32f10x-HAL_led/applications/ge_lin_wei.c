#include <rtthread.h>
#include <rtdevice.h>
#include "config.h"
gelinwei_info_t gelinwei_info;
     
/*
					多功能灯检测 当前是箭头还是圆饼
					
		|-------------------------------------------|
		| LG  |  LR  |  LY   |    AG  |  AR  | AY   |      
		|--------------------|----------------------|
		|     左箭头				 |       满盘         |  
		|    通道0xc1        | 			通道0xc0        |  
	 <--------------------------------------------->
		| AG  |  AR  |  AY   |    RG  |  RR  | RY   |      
		|--------------------|----------------------|
		|     直箭头				 |       满盘         |  
		|    通道0xc2        | 			通道0xc0        |  
	 <--------------------------------------------->
		| RG  |  RR  |  RY   |    AG  |  AR  | AY   |      
		|--------------------|----------------------|
		|     右箭头				 |       满盘           |  
		|    通道0xc3        | 			通道0xc0        |  
		|--------------------|----------------------|
		
*/

static rt_uint8_t  mux_detection(rt_uint8_t dir) 
{
	static rt_uint8_t countdown_direction_tmp = 0;	
	countdown_direction_tmp = dir;
//	 if(AHEAD_R||AHEAD_Y||AHEAD_G)
//		{
//			if((dir == 0xc1)||(dir == 0xc3))
//				countdown_direction_tmp = 0xc0;
//		}
//	else if(RIGHT_R||RIGHT_Y||RIGHT_G)
//		{
//			if(dir == 0xc2)
//			countdown_direction_tmp = 0xc0;
//		}
	
		if((dir == 0xc1)||(dir == 0xc2)||(dir == 0xc3))
		{
			countdown_direction_tmp = 0xc0;
		}
		
		
	return countdown_direction_tmp;
}

static void parse(config_t config)
{
	rt_uint8_t mm=0;
	rt_uint8_t jj = 0;     	//用于将信息码密文放入Sig_buf1
	rt_uint8_t hh = 0;    	//用于将信息码原文放入Sig_buf2
	rt_uint8_t sig_cat = 0;
	rt_uint8_t key_B = 0;
	rt_uint8_t key_C = 0;
	rt_uint8_t M1M1 = 0;
	rt_uint8_t M2M2 = 0;
	rt_uint8_t P_cat = 0;	//P'和Q'在密文中的位置
	rt_uint8_t Q_cat = 0; 
	rt_uint8_t P_p = 0;//p'	
	rt_uint8_t Q_q= 0; //q'
	rt_uint8_t enc_data[128]={0};//密文 含P' 和  Q'
	rt_uint8_t enc_data_t[128]={0};//密文 不含P' 和  Q'
	rt_uint8_t buf_len=0;//接收的包长度
	rt_uint8_t dir = 0;
	rt_uint8_t dir_mux = 0;
	rt_uint8_t sig_data_len=0,p_pos=0; //临界状态值
	buf_len=config.rx_counter;
	sig_data_len =  buf_len - 9;
			if(config.rx_buffer[4]==(buf_len-5))//判断长度
			{
				rt_memcpy(enc_data,config.rx_buffer,config.rx_counter);
				rt_memset(config.rx_buffer,0,config.rx_counter);

				if(0x7e==enc_data[5])//命令字
				{
					key_B=enc_data[6];//密钥B
					M1M1=(key_B & 0xf0) | (key_B & 0xf0) >> 4;
					M2M2=(key_B & 0x0f )| (key_B & 0x0f) << 4;
					P_cat = (M1M1 & 0x0f) % 8;	 //P'位置，密文从左到右第P_cat位
					Q_cat = (M2M2 & 0x0f) % 8;	 //Q'位置，密文从右到左第Q_cat位
			
				if((sig_data_len - Q_cat) >	P_cat 	)
				{
					P_p=enc_data[P_cat+7]^M1M1;	
					p_pos = P_cat+7;
				}
				else 
				{
					P_p=enc_data[P_cat+8]^M1M1;
					p_pos = P_cat+8;
				}
					
					Q_q=enc_data[buf_len-2-Q_cat]^M2M2;
					key_C = (P_p&0x0f)<<4 | (Q_q&0xf0)>>4;
						for(jj = 7;jj < buf_len-1;jj++)	  //从第八个数开始存放，buf_cat2-1是因为要减去校验和
						{					
							if(((p_pos) != jj) && ((buf_len-2-Q_cat) != jj))	 //去掉P'和Q'
							{
								enc_data_t[sig_cat++] = enc_data[jj];
							}
						}
						for(hh = 0;hh < sig_cat;hh++)
						{
							gelinwei_info.org_data[hh] = enc_data_t[hh] ^ key_C;
						}
												gelinwei_info.dir=gelinwei_info.org_data[0];
												gelinwei_info.cmd=gelinwei_info.org_data[1];
												gelinwei_info.countdown_type=gelinwei_info.org_data[2];
												gelinwei_info.reflash_flag=gelinwei_info.org_data[3];
												//test
												//config.countdown_direction=0xb0;
												//config.singal_direction =0xc1;
											dir     = config.singal_direction;//
											dir_mux = mux_detection(config.singal_direction);
											if(config.countdown_direction ==gelinwei_info.dir )
											{
														while(1)
															{
																/*判断方向，获取有效信息*/
																if((dir == gelinwei_info.org_data[mm+4])||(dir_mux==gelinwei_info.org_data[mm+4]))
																{
																	gelinwei_info.addr=gelinwei_info.org_data[mm+4];
																	gelinwei_info.data_h=((gelinwei_info.org_data[mm+4+2]&0xf0)>>4)*1000+(((gelinwei_info.org_data[mm+4+2]&0x0f))*100);
																	gelinwei_info.data_l=((gelinwei_info.org_data[mm+4+3]&0xf0)>>4)*10+((gelinwei_info.org_data[mm+4+3]&0x0f));
																	gelinwei_info.color=gelinwei_info.org_data[mm+4+1];
																	
																	if((0x05==gelinwei_info.reflash_flag))//刷新码
																		{
																			//reflash the countdonw data
																			countdown.cur_tmp=gelinwei_info.data_h;
																			countdown.cur_tmp=(countdown.cur_tmp<<8)|gelinwei_info.data_l;	
																		}
																	else if(0x03==gelinwei_info.reflash_flag)
																			{
																				// no reflash
																			}	
																	
																	if(0xd1==gelinwei_info.color)//判断显示颜色  如果命令字为关屏 则不显示  
																	{
																		countdown.cur_num_g = countdown.cur_tmp;
																		countdown.flag_g = 1;
																		countdown.mode_g = 1;
																		countdown.cur_mode    = GREEN_MODE;
																	}
																	else if(0xd3==gelinwei_info.color)
																	{
																		countdown.cur_num_r = countdown.cur_tmp;
																		countdown.flag_r = 1;
																		countdown.mode_r = 1;
																		countdown.cur_mode    = RED_MODE;
																	}
																	else if(0xd2==gelinwei_info.color)
																	{
																		countdown.cur_num_y = countdown.cur_tmp;
																		countdown.flag_y = 1;
																		countdown.mode_y = 1;
																		countdown.cur_mode    = YELLOW_MODE;
																	}
																	else if(0xd0==gelinwei_info.color)
																	countdown.cur_mode    = BLACK_MODE;																					
																

																	if(1==gelinwei_info.cmd)// 命令字
																				countdown.cur_mode    = BLACK_MODE;
																	else if(2==gelinwei_info.cmd)//显示H
																				countdown.cur_mode    = H_MODE;
																			
																	if(0x7c == gelinwei_info.countdown_type)
																			 countdown.type=0;//1秒刷新一次数据
																	else if(0x7d == gelinwei_info.countdown_type)
																			{
																				countdown.type=1;//发送一个总的数据 由倒计时自行递减
																				//countdown.sync = 0;
																			}
																			break;//break out the while
																}	
																else
																{
																	mm += 4;
																	if(mm > hh)
																	{
																		break;//break out the while
																	}									
																}	
														 }
										}	
												rt_memset(&gelinwei_info,0,sizeof(gelinwei_info));
						}
				 }
	}
void ge_lin_wei_parse(config_t config)
{
		rt_uint8_t rx_buf[512]={0};//由256增大值512
		rt_uint16_t rx_len=0;
		rt_uint16_t loop_i=0;
		rt_memcpy(rx_buf,config.rx_buffer,config.rx_counter);
		rx_len=config.rx_counter;
		rt_memset(config.rx_buffer,0,config.rx_counter);
		for(loop_i=0;loop_i<rx_len;loop_i++)
		{
			if((0x2c==rx_buf[loop_i])&&(0x2c==rx_buf[loop_i+1])&&(0x2c==rx_buf[loop_i+2])&&(0x2c==rx_buf[loop_i+3]))
			{
				if(rx_len-loop_i>9)
				config.rx_counter=rx_buf[loop_i+4]+5;
				rt_memcpy(config.rx_buffer,&rx_buf[loop_i],config.rx_counter);
				parse(config);
			}
		}
		
}


