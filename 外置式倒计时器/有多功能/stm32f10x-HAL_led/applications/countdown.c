#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "config.h"
#include "font.h"


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t countdown_stack[ 512 ];
/* ???TCB??? */
static struct rt_thread countdown_thread;
static struct rt_timer timer_count;
static rt_uint8_t countdown_type = 0;
countdown_t countdown;

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;
    /* 获得延时经过的 tick 数 */
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
    /* 获得当前时间 */
    delta = SysTick->VAL;
    /* 循环获得当前时间，直到达到指定的时间后退出循环 */
    while (delta - SysTick->VAL< us);
}

static void timeout(void* parameter)
{
	
	if(1 == countdown_type)
	{
		
		if((countdown.sync==5)&&(countdown.cur_num>=2))
			{
					countdown.cur_num--;
					countdown.sync = 0;
			}
		else if(countdown.cur_num==1)
			{
//				if((config_init.motor_or_people==1)&&(config_init.study_or_comm==1))
//				{
//					if((countdown.sync>=3)&&(countdown.cur_mode==GREEN_MODE))	
//					{
//						countdown.cur_mode=4;
//						countdown.cur_num--;
//					}
//				}
//				else if(countdown.sync>=4)
//				{
//					countdown.cur_mode=4;
//					countdown.cur_num--;
//				}
				
		if(countdown.sync>=3)
				{
					countdown.cur_mode=4;
					countdown.cur_num--;
				}
				countdown_type = 0;
			}
		countdown.sync++;
	}
	
}
int rt_timer_count_init(void)
{
    /* 初始化定时器 */
    rt_timer_init(&timer_count, "timer1",  /* 定时器名字是 timer_count */
                    timeout, /* 超时时回调的处理函数 */
                    RT_NULL, /* 超时函数的入口参数 */
                    20, /* 定时长度，以OS Tick为单位，即10个OS Tick */
                    RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
		rt_timer_start(&timer_count); 
    return 0;
}
INIT_ENV_EXPORT(rt_timer_count_init);  

rt_uint32_t up[16],down[16];
rt_uint8_t three_buf[32][4];

rt_uint32_t decode(rt_uint8_t data1,rt_uint8_t data2,rt_uint8_t data3,rt_uint8_t data4)
 {
		rt_uint32_t ret = 0;	 
		ret |=(data4&0xf0)<<24;
		ret |=(data3&0xf0)<<20;
		ret |=(data2&0xf0)<<16; 
		ret |=(data1&0xf0)<<12; 
		ret |=((data4&0x0f)<<4)<<8;
		ret |=((data3&0x0f)<<4)<<4;
		ret |=((data2&0x0f)<<4);
		ret |=(data1&0x0f);
	  return ret;
 }
//一次写32个点的数据
void bank32(rt_uint32_t data_up,rt_uint32_t data_down,rt_uint8_t color)
{
			int i ,color1 ,color2;
	    if (color == RED_MODE)
				{
					color1=RD1_PIN;
					color2=RD2_PIN;
				}
			else if (color == GREEN_MODE)
				{
					color1=BD1_PIN;
					color2=BD2_PIN;
				}
			else if (color == YELLOW_MODE)
				{
					color1=GD1_PIN;
					color2=GD2_PIN;
				}		
			else if (color == BLACK_MODE)
				{
					rt_pin_write(OE_PIN, PIN_HIGH);	
					return;
				}
			
			for(i=0;i<32;i++)
			{  	
			   rt_pin_write(CLK_PIN, PIN_LOW);		
				 rt_pin_write(color1, data_up&0x00000001);
				 rt_pin_write(color2, data_down&0x00000001);
			   rt_pin_write(CLK_PIN, PIN_HIGH);		
				 data_up=data_up>>1;
			   data_down=data_down>>1;			
			}		
}

void Clear_Display(void)//关显示
{
	int i , j ;
	 rt_pin_write(LAT_PIN, PIN_LOW);
	 rt_pin_write(OE_PIN,  PIN_HIGH);
	 for(j=0;j<32;j++)
	 {	
			for(i=0;i<16;i++)
			{ 
			   rt_pin_write(CLK_PIN, PIN_LOW);
				 rt_pin_write(RD1_PIN, PIN_LOW);
				 rt_pin_write(RD2_PIN, PIN_LOW);
				 rt_pin_write(GD1_PIN, PIN_LOW);
				 rt_pin_write(GD2_PIN, PIN_LOW);
				 rt_pin_write(BD1_PIN, PIN_LOW);
				 rt_pin_write(BD2_PIN, PIN_LOW);
			   rt_pin_write(CLK_PIN, PIN_HIGH);
			}				
		}
	 rt_pin_write(LAT_PIN, PIN_HIGH);
	 rt_pin_write(OE_PIN, PIN_LOW);	
	rt_thread_delay(RT_TICK_PER_SECOND/5);
}

void two_bit_decode(const rt_uint8_t frame[32][2],const rt_uint8_t frame_t[32][2])
{

	//-------------------------------left------------------------
	up[0]=decode(frame[7][0],frame[6][0],frame[5][0],frame[4][0]);
	down[0]=decode(frame[15][0],frame[14][0],frame[13][0],frame[12][0]);
	up[1]=decode(frame[7][1],frame[6][1],frame[5][1],frame[4][1]);
	down[1]=decode(frame[15][1],frame[14][1],frame[13][1],frame[12][1]);
	up[2]=decode(frame[3][0],frame[2][0],frame[1][0],frame[0][0]);
	down[2]=decode(frame[11][0],frame[10][0],frame[9][0],frame[8][0]);
	up[3]=decode(frame[3][1],frame[2][1],frame[1][1],frame[0][1]);
	down[3]=decode(frame[11][1],frame[10][1],frame[9][1],frame[8][1]);
	up[4]=decode(frame[23][0],frame[22][0],frame[21][0],frame[20][0]);
	down[4]=decode(frame[31][0],frame[30][0],frame[29][0],frame[28][0]);
	up[5]=decode(frame[23][1],frame[22][1],frame[21][1],frame[20][1]);
	down[5]=decode(frame[31][1],frame[30][1],frame[29][1],frame[28][1]);
	up[6]=decode(frame[19][0],frame[18][0],frame[17][0],frame[16][0]);
	down[6]=decode(frame[27][0],frame[26][0],frame[25][0],frame[24][0]);
	up[7]=decode(frame[19][1],frame[18][1],frame[17][1],frame[16][1]);
	down[7]=decode(frame[27][1],frame[26][1],frame[25][1],frame[24][1]);
	//-------------------------------right------------------------
	up[8]=decode(frame_t[7][0],frame_t[6][0],frame_t[5][0],frame_t[4][0]);
	down[8]=decode(frame_t[15][0],frame_t[14][0],frame_t[13][0],frame_t[12][0]);
	up[9]=decode(frame_t[7][1],frame_t[6][1],frame_t[5][1],frame_t[4][1]);
	down[9]=decode(frame_t[15][1],frame_t[14][1],frame_t[13][1],frame_t[12][1]);
	up[10]=decode(frame_t[3][0],frame_t[2][0],frame_t[1][0],frame_t[0][0]);
	down[10]=decode(frame_t[11][0],frame_t[10][0],frame_t[9][0],frame_t[8][0]);
	up[11]=decode(frame_t[3][1],frame_t[2][1],frame_t[1][1],frame_t[0][1]);
	down[11]=decode(frame_t[11][1],frame_t[10][1],frame_t[9][1],frame_t[8][1]);
	up[12]=decode(frame_t[23][0],frame_t[22][0],frame_t[21][0],frame_t[20][0]);
	down[12]=decode(frame_t[31][0],frame_t[30][0],frame_t[29][0],frame_t[28][0]);
	up[13]=decode(frame_t[23][1],frame_t[22][1],frame_t[21][1],frame_t[20][1]);
	down[13]=decode(frame_t[31][1],frame_t[30][1],frame_t[29][1],frame_t[28][1]);
	up[14]=decode(frame_t[19][0],frame_t[18][0],frame_t[17][0],frame_t[16][0]);
	down[14]=decode(frame_t[27][0],frame_t[26][0],frame_t[25][0],frame_t[24][0]);
	up[15]=decode(frame_t[19][1],frame_t[18][1],frame_t[17][1],frame_t[16][1]);
	down[15]=decode(frame_t[27][1],frame_t[26][1],frame_t[25][1],frame_t[24][1]);
	
}


void three_bit_decode(const rt_uint8_t frame[32][4])
{
	//-------------------------------32x16 left-----------------------
	up[0]=decode(frame[7][2],frame[6][2],frame[5][2],frame[4][2]);
	down[0]=decode(frame[15][2],frame[14][2],frame[13][2],frame[12][2]);
	up[1]=decode(frame[7][3],frame[6][3],frame[5][3],frame[4][3]);
	down[1]=decode(frame[15][3],frame[14][3],frame[13][3],frame[12][3]);
	up[2]=decode(frame[3][2],frame[2][2],frame[1][2],frame[0][2]);
	down[2]=decode(frame[11][2],frame[10][2],frame[9][2],frame[8][2]);
	up[3]=decode(frame[3][3],frame[2][3],frame[1][3],frame[0][3]);
	down[3]=decode(frame[11][3],frame[10][3],frame[9][3],frame[8][3]);
	up[4]=decode(frame[23][2],frame[22][2],frame[21][2],frame[20][2]);
	down[4]=decode(frame[31][2],frame[30][2],frame[29][2],frame[28][2]);
	up[5]=decode(frame[23][3],frame[22][3],frame[21][3],frame[20][3]);
	down[5]=decode(frame[31][3],frame[30][3],frame[29][3],frame[28][3]);
	up[6]=decode(frame[19][2],frame[18][2],frame[17][2],frame[16][2]);
	down[6]=decode(frame[27][2],frame[26][2],frame[25][2],frame[24][2]);
	up[7]=decode(frame[19][3],frame[18][3],frame[17][3],frame[16][3]);
	down[7]=decode(frame[27][3],frame[26][3],frame[25][3],frame[24][3]);
	//-------------------------------32x16 right------------------------
	up[8]=decode(frame[7][0],frame[6][0],frame[5][0],frame[4][0]);
	down[8]=decode(frame[15][0],frame[14][0],frame[13][0],frame[12][0]);
	up[9]=decode(frame[7][1],frame[6][1],frame[5][1],frame[4][1]);
	down[9]=decode(frame[15][1],frame[14][1],frame[13][1],frame[12][1]);
	up[10]=decode(frame[3][0],frame[2][0],frame[1][0],frame[0][0]);
	down[10]=decode(frame[11][0],frame[10][0],frame[9][0],frame[8][0]);
	up[11]=decode(frame[3][1],frame[2][1],frame[1][1],frame[0][1]);
	down[11]=decode(frame[11][1],frame[10][1],frame[9][1],frame[8][1]);
	up[12]=decode(frame[23][0],frame[22][0],frame[21][0],frame[20][0]);
	down[12]=decode(frame[31][0],frame[30][0],frame[29][0],frame[28][0]);
	up[13]=decode(frame[23][1],frame[22][1],frame[21][1],frame[20][1]);
	down[13]=decode(frame[31][1],frame[30][1],frame[29][1],frame[28][1]);
	up[14]=decode(frame[19][0],frame[18][0],frame[17][0],frame[16][0]);
	down[14]=decode(frame[27][0],frame[26][0],frame[25][0],frame[24][0]);
	up[15]=decode(frame[19][1],frame[18][1],frame[17][1],frame[16][1]);
	down[15]=decode(frame[27][1],frame[26][1],frame[25][1],frame[24][1]);
	
}

rt_uint32_t three_bit_data_decode(const rt_uint8_t frame[32][2],const rt_uint8_t frame1[32][2],rt_uint8_t i)
{
	  rt_uint32_t data = 0;
		data =first_one_3bit[i]<<24;
		data|=(frame[i][0])<<16;
		data|=(((frame[i][1]&0x0f))|((frame1[i][0]&0x0f)<<4))<<8;
		data|=((frame1[i][1]&0x0f)<<4)|((frame1[i][0]&0xf0)>>4);
	  return data;
}
void dispaly(rt_uint8_t color)
{			
			rt_uint8_t i = 0 ;
			rt_pin_write(LAT_PIN, PIN_LOW);	
			rt_hw_us_delay(2);
			for(i=0;i<16;i++)
			bank32(up[i],down[i],color);
			rt_hw_us_delay(2);
			rt_pin_write(LAT_PIN, PIN_HIGH);
			rt_pin_write(OE_PIN, PIN_LOW);	
}
void three_bit_display(rt_uint8_t value,rt_uint8_t color)
{
	rt_uint8_t i = 0 ;
	rt_uint8_t num_shi,num_ge;
	rt_uint32_t data = 0;	
	num_shi = (value%100)/10;
	num_ge  =  value%10 ;
	
	if (value >= 100)
	{
		for (i=0;i<32;i++)
		{
			data=three_bit_data_decode(&s[num_shi][0],&s[num_ge][0],i);
			three_buf[i][0]=data>>24;
			three_buf[i][1]=(data&0x00ff0000)>>16;
			three_buf[i][2]=(data&0x0000ff00)>>8;
			three_buf[i][3]=data&0x000000ff;
		}	
		three_bit_decode(three_buf);
		dispaly(color);
	}
}

void two_bit_display(rt_uint8_t value,rt_uint8_t color)
{
	rt_uint8_t num_shi,num_ge;
	num_shi = value%10;
	num_ge  = value/10;
	two_bit_decode(&p[num_shi][0],&p[num_ge][0]);//十位
	dispaly(color);
}

void one_bit_display(rt_uint8_t value,rt_uint8_t color)
{
	rt_uint8_t num_ge;
	num_ge  = value%10-1;
	three_bit_decode(&q[num_ge][0]);
	dispaly(color);
}
void H_Display(void)//H显示
{
	three_bit_decode(&q[9][0]);
	dispaly(YELLOW_MODE);
	rt_thread_delay(RT_TICK_PER_SECOND/5);
}
static void Set_LED_Display(rt_uint16_t value,rt_uint8_t color,rt_uint8_t blind)//倒计时值 倒计时颜色
{

	if(value==0)
	{
		Clear_Display();
		return ;
	}
	
	if(value >= 100)
		three_bit_display( value, color);
	else if((value>=10)&&(value < 100))
		two_bit_display( value, color);
	else 
		one_bit_display (value,color);

	if(0==blind)
	{
		rt_pin_write(OE_PIN, 0);
		rt_thread_delay(RT_TICK_PER_SECOND/5);
	}
	else
	{
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		rt_pin_write(OE_PIN, 1);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
	}
}

static rt_uint8_t light_changed(void)
{
	
	if(config_init.motor_or_people == 0)//人行
	{
		if(LEFT_R==1)//人行红灯
			return 1; 
		else 				 //人行绿灯
			return 2;
	}
	else if(config_init.motor_or_people == 1)//机动
	{
		if((AHEAD_R == 1)||(LEFT_R == 1)||(RIGHT_R == 1))//机动红灯
			return 3;
		else if ((AHEAD_Y == 1)||(LEFT_Y == 1)||(RIGHT_Y == 1))
			return 5;		//机动黄灯
		else 
			return 4;		//机动绿灯
	}
	else return 0;
		
}


static void rt_hw_countdown(countdown_t CountDown)
{
		static  rt_uint8_t light_flg =0 ;
				light_flg = light_changed();
			
				if((light_flg == 1)||(light_flg == 3))// 红灯
				{	
					if(countdown.mode_r ==1)
					{	
					 if((1==MOTRO_OR_PEOPLE)&&(light_flg == 3))//机动最大只能显示199秒
							{		
								countdown_type = countdown.type ;
								if(countdown.flag_r)
								{
									Clear_Display();
									countdown.mode_g = 0 ;
									countdown.mode_y = 0 ;
									countdown.sync = 0;
									countdown.cur_num=countdown.cur_num_r;
									countdown.flag_r = 0;
								}
								if(countdown.cur_num<=199)
								Set_LED_Display(countdown.cur_num,RED_MODE,countdown.blind);
								else Clear_Display();
							}
							else Clear_Display();
					}		
				 else Clear_Display();					
				}
				else if(((light_flg == 2)||(light_flg == 4))&&R_LED)//绿灯
				{
					if(countdown.mode_g ==1)
					{		
						 if((1==MOTRO_OR_PEOPLE)&&(light_flg == 4))
							{
								countdown_type = countdown.type ;
								if(countdown.flag_g)
								{
									Clear_Display();
									countdown.mode_r = 0 ;
									countdown.mode_y = 0 ;
									countdown.sync = 0;
									countdown.cur_num=countdown.cur_num_g;
									countdown.flag_g = 0;
								}
								if(countdown.cur_num<=199)
									Set_LED_Display(countdown.cur_num,GREEN_MODE,countdown.blind);
								else Clear_Display();
							}
							else Clear_Display();
					}
					else Clear_Display();
				}
				else if((light_flg == 5)&&R_LED)//黄灯
				{
					countdown_type = countdown.type ;
					if(countdown.mode_y ==1)
					{
						if((1==MOTRO_OR_PEOPLE))
						{
								if(countdown.flag_y)
								{
									Clear_Display();
									countdown.mode_r = 0 ;
									countdown.mode_g = 0 ;
									countdown.sync = 0;
									countdown.cur_num=countdown.cur_num_y;
									countdown.flag_y = 0;
								}
								if(countdown.cur_num<=199)
									Set_LED_Display(countdown.cur_num,YELLOW_MODE,countdown.blind);	
								else Clear_Display();	
						}
						else Clear_Display();
					}
				}
				else if(BLACK_MODE==CountDown.cur_mode)
				{
					Clear_Display();
				}
				else if(H_MODE==CountDown.cur_mode)
				{
					H_Display();
				}
				else
				{
					rt_thread_delay(RT_TICK_PER_SECOND/20);
				}		
}


int rt_hw_countdown_init(void)
{
	 rt_pin_mode(RD1_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(RD2_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(GD1_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(GD2_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(BD1_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(BD2_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(CLK_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(LAT_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(OE_PIN,  PIN_MODE_OUTPUT);
	 /*set the io level*/    
	 rt_pin_write(RD1_PIN, PIN_LOW);
	 rt_pin_write(RD2_PIN, PIN_LOW);
	 rt_pin_write(GD1_PIN, PIN_LOW);
	 rt_pin_write(GD2_PIN, PIN_LOW);
	 rt_pin_write(BD1_PIN, PIN_LOW);
	 rt_pin_write(BD2_PIN, PIN_LOW);
	 rt_pin_write(LAT_PIN, PIN_LOW);
	 rt_pin_write(CLK_PIN, PIN_LOW);
   rt_pin_write(OE_PIN, PIN_HIGH);	
		return 0;
}

INIT_ENV_EXPORT(rt_hw_countdown_init);  


static void countdown_thread_entry(void *parameter)
{

	countdown.cur_mode=4;
	countdown.cur_num=0;
	countdown.blind=0;
	countdown.type=0;
	
    while (1)
    {
			rt_hw_countdown(countdown);//每秒刷新倒计时			
    }
}

int countdown_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&countdown_thread,
                            "countdown",
                            countdown_thread_entry,
                            RT_NULL,
                            (rt_uint8_t *)&countdown_stack[0],
                            sizeof(countdown_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&countdown_thread);
    }
    return 0;
}
 INIT_APP_EXPORT(countdown_init);


