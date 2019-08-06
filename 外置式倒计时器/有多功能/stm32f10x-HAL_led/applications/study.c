#include <rtthread.h>
#include <rtdevice.h>
#include "config.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t study_stack[ 256 ];
static struct rt_thread study_thread;
static struct rt_timer timer_study;


//信号灯顺序 绿灯->绿闪->黄->红->绿       绿灯green 红灯red 黄灯yellow 

//初次上电后先学习2-3个周期然后再开始显示倒计时！
/*****************************************************************motor*********************************************************/
rt_uint8_t red_flag=0;
rt_uint8_t green_flag=0;
rt_uint8_t yellow_flag=0;

rt_uint16_t red_counter=0;
rt_uint16_t green_counter=0;
rt_uint16_t yellow_counter=0;

rt_uint8_t red_time[2]={2,0};
rt_uint8_t green_time[2]={2,0};
rt_uint8_t yellow_time[2]={2,0};


rt_uint8_t r_t =0;
rt_uint8_t g_t =0;
rt_uint8_t y_t =0;



/*****************************************************************people**********************************************************/
rt_uint8_t people_red_flag=0;
rt_uint8_t people_green_flag=0;

rt_uint16_t people_red_counter=0;
rt_uint16_t people_green_counter=0;

rt_uint8_t people_red_time[2]={2,0};
rt_uint8_t people_green_time[2]={2,0};

rt_uint8_t p_r_t =0;
rt_uint8_t p_g_t =0;


/************************************************************motor_or_people*******************************************************/
rt_uint8_t m_or_p =0;


static void timer_out(void* parameter)
{
	if(0==m_or_p)//行人信号学习
	{
			if(people_red_flag)
			{
				people_red_counter++;//红灯计时
			}
			else if(people_green_flag)
			{
				people_green_counter++; //绿灯计时
			}
	}
	else if(1==m_or_p)//机动信号学习
	{
		if(red_flag)
		{
			red_counter++;//红灯计时
		}
		else if(green_flag)
		{
			green_counter++; //绿灯计时
		}
		else if(yellow_flag)
		{
			yellow_counter++;//黄灯计时
		}
	}
}

int rt_timer_study_init(void)
{
      /* 初始化定时器 */
    rt_timer_init(&timer_study, "study", /* 定时器名字是 timer_study */
                    timer_out, /* 超时时回调的处理函数 */
                    RT_NULL,  /* 超时函数的入口参数 */
                    10, /* 定时长度，以OS Tick为单位，即10个OS Tick */
                    RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
		rt_timer_start(&timer_study); 
    return 0;
}


/**********************************************机动********************************************************/
void rt_hw_motor_stduy(void) //机动学习用的是主控板300DJS-RYG-CN V1.0  AR AY AG 端子接线
{
if(AHEAD_R==1&&AHEAD_G==0&&AHEAD_Y==0)//现在是红灯
	{		
		if(yellow_flag)
		{
			yellow_time[1]=yellow_time[0];		
			yellow_time[0]=(yellow_counter+1)/10;//黄灯时间保存
		}
		y_t = 0;
		yellow_flag=0;//关闭黄灯计时
		red_flag=1;//计时标志位置位
		yellow_counter=0;
		if(((red_time[1]-red_time[0])==0)&&(r_t==0))
					{
							countdown.blind=0;
							countdown.cur_mode=RED_MODE;
							countdown.cur_num_r=red_time[0];
							countdown.mode_r = 1;
							countdown.flag_r = 1;
							countdown.type=1;//自动倒计时
					//		countdown.sync = 0;
							r_t=1;
							
					}
			else if(((red_time[1]-red_time[0])!=0))
				{
					r_t = 0;
					countdown.cur_mode=BLACK_MODE;
				}
		
	}
	
else if(AHEAD_R==0&&AHEAD_G==1&&AHEAD_Y==0)//现在是绿灯
	{
		if(red_flag)
		{
			red_time[1]=red_time[0];
			red_time[0]=(red_counter+1)/10;//保存红灯时间
		}
		r_t = 0;
		red_flag=0;//红灯计数标志位清零 计时结束
		green_flag=1;//计时标志位置位	
		red_counter=0;//红灯计数器清零
		if(((green_time[1]-green_time[0])==0)&&(g_t==0))
		{
				countdown.blind=0;
				countdown.cur_mode=GREEN_MODE;
				countdown.cur_num_g=green_time[0];
				countdown.mode_g = 1;
				countdown.flag_g = 1;
				countdown.type=1;//自动倒计时
				//countdown.sync = 0;
				g_t=1;
				
		}
		else if(((green_time[1]-green_time[0])!=0))
		{
			g_t = 0;
			countdown.cur_mode=BLACK_MODE;
		}
	}
	else if(AHEAD_R==0&AHEAD_G==0&&AHEAD_Y==1)//现在是黄灯		
	{
		if(green_flag)
		{
			green_time[1]=green_time[0];
			green_time[0]=(green_counter+1)/10;//保存绿灯时间
		}
		g_t = 0;
		green_flag=0;//停止绿灯计时
		yellow_flag=1;//绿闪计时标志位置位
		green_counter=0;//绿灯计时器清零
		if(((yellow_time[1]-yellow_time[0])==0)&&(y_t==0))
		{
				countdown.blind=0;
				countdown.cur_mode=YELLOW_MODE;
				countdown.cur_num_y=yellow_time[0];
				countdown.mode_y = 1;
				countdown.flag_y = 1;
				countdown.type=1;
				//countdown.sync = 0;
				y_t = 1;
		}
		else if(((yellow_time[1]-yellow_time[0])!=0))
		{
			y_t = 0;
			countdown.cur_mode=BLACK_MODE;
		}
	}
	else {};
}

/**********************************************人行********************************************************/
void  rt_hw_people_stduy(void) //行人学习用的是主控板300DJS-RYG-CN V1.0  LR  LG 端子接线
{
	if(LEFT_R==1&&LEFT_G==0)//现在是红灯
	{		
		if(people_green_flag)
		{
			people_green_time[1]=people_green_time[0];		
		  	people_green_time[0]=(people_green_counter+1)/10;//绿灯时间保存
		}
		p_g_t = 0;
		people_green_flag=0;//关闭绿灯计时
		people_red_flag=1;//计时标志位置位
		people_green_counter=0;
		if(((people_red_time[1]-people_red_time[0])==0)&&(p_r_t==0))
					{
							countdown.blind=0;
							countdown.cur_mode=RED_MODE;
							countdown.cur_num_r=people_red_time[0];
							countdown.flag_r = 1;
							countdown.mode_r = 1;
							countdown.type=1;//自动倒计时
							//countdown.sync = 0;
							p_r_t=1;						
					}
			else if(((people_red_time[1]-people_red_time[0])!=0))
				{
					p_r_t = 0;
					countdown.cur_mode=BLACK_MODE;
				}
	}
else if(LEFT_R==0&&LEFT_G==1)//现在是绿灯
	{
		if(people_red_flag)
		{
			people_red_time[1]=people_red_time[0];
			people_red_time[0]=(people_red_counter+1)/10;//保存红灯时间
		}
		 p_r_t = 0;
		 people_red_flag=0;//红灯计数标志位清零 计时结束
		 people_green_flag=1;//计时标志位置位	
		 people_red_counter=0;//红灯计数器清零
		if(((people_green_time[1]-people_green_time[0])==0)&&(p_g_t==0))
		{
				countdown.blind=0;
				countdown.cur_mode=GREEN_MODE;
				countdown.cur_num_g=people_green_time[0];
				countdown.flag_g = 1;
				countdown.mode_g = 1;
				countdown.type=1;//自动倒计时
				//countdown.sync = 0;
				p_g_t=1;
		}
		else if(((people_green_time[1]-people_green_time[0])!=0))
		{
			p_g_t = 0;
			countdown.cur_mode=BLACK_MODE;
		}
	}
	else {};
}


/**********************************************************thread*********************************************************************/


static void study_thread_entry(void *parameter)
{
		m_or_p=MOTRO_OR_PEOPLE;
    while (1)
    {  
			if(1==STUDY_OR_COMM)
			{
				if(0==m_or_p)//行人
					rt_hw_people_stduy();
				else if(1==m_or_p)//机动
					rt_hw_motor_stduy();
			}
				  rt_thread_delay(RT_TICK_PER_SECOND/50);
    }
}

int study_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&study_thread,
                            "study",
                            study_thread_entry,
                            RT_NULL,
                            (rt_uint8_t *)&study_stack[0],
                            sizeof(study_stack),
                            19,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&study_thread);
    }
    return 0;
}
 INIT_ENV_EXPORT(rt_timer_study_init);
 INIT_APP_EXPORT(study_init);
	


