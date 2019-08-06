#include <rtthread.h>
#include <rtdevice.h>
#include "config.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rcv_process_stack[ 4096 ];
static struct rt_thread rcv_process_thread;

/* 串口接收事件标志 */
#define UART_CNT_RX_EVENT (1 << 0)
#define UART_CNT_RXIDLE_EVENT (1 << 1)
/* 事件控制块 */
static struct rt_event rcv_event;
/* 设备句柄 */
static rt_device_t uart_device = RT_NULL;

/* 回调函数 */
static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
	  rt_uint16_t event_flg;
    /* 发送事件 */
	  if(size == 0){
			event_flg = UART_CNT_RXIDLE_EVENT;
		}
		else{
			event_flg = UART_CNT_RX_EVENT;
		}
		rt_event_send(&rcv_event, event_flg);
    return RT_EOK;
}

rt_err_t uart_open(const char *name)
{
    rt_err_t res;
    /* 查找系统中的串口设备 */
    uart_device = rt_device_find(name);
    /* 查找到设备后将其打开 */
    if (uart_device != RT_NULL)
    {
        res = rt_device_set_rx_indicate(uart_device, uart_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return -RT_ERROR;
        }
        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR |
                             RT_DEVICE_FLAG_INT_RX );
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",name,res);
            return -RT_ERROR;
        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n",name);
        return -RT_ERROR;
    }
    /* 初始化事件对象 */
    rt_event_init(&rcv_event, "rcv_event", RT_IPC_FLAG_FIFO);
    return RT_EOK;
}

rt_uint16_t counter=0;
rt_int8_t uart_get_cnt_data(void)
{
    rt_uint32_t e;
	  rt_int8_t ret = -1;
		
	  /* 接收事件 */
    rt_event_recv(&rcv_event, UART_CNT_RX_EVENT|UART_CNT_RXIDLE_EVENT,RT_EVENT_FLAG_OR |
                      RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
	
	  if(e & UART_CNT_RX_EVENT){
			/* 读取 1 字节数据 */
			while (rt_device_read(uart_device, 0, &config_init.rx_buffer[counter], 1) ==1)
			{
					if((counter++)>=512)
						counter = 0;
			}

		}
		
		if(e & UART_CNT_RXIDLE_EVENT){
		
			config_init.rx_counter = counter;
			counter = 0;
			ret = 0;
		}
		return ret;
}

void call_rcv_parse (void (*callfuct)(config_t config_init))
{
	if(0==STUDY_OR_COMM)//判断通讯型还是学习型
	callfuct(config_init);
}
void subpackage(config_t config_init)
{
	if(PROTOCOL==1)//判断协议类型  原理是可以自定识别  可靠方案还是拨码
	{
		call_rcv_parse(ge_lin_wei_parse);
	}
	else 
	{
		if((0x55 == config_init.rx_buffer[0])&&(0xaa == config_init.rx_buffer[1]))
		call_rcv_parse(gat_2014_parse);
		else if(0xfe == config_init.rx_buffer[0])
		call_rcv_parse(gat_2004_parse);
	}
}


static void rcv_process_thread_entry(void *parameter)
{
	  if(RT_EOK != uart_open("uart2")) return;
						rt_hw_uart_init();

    while (1)
    {  
				if(uart_get_cnt_data() == 0)
					subpackage(config_init);
    }
}

int rcv_process_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&rcv_process_thread,
                            "rcv_process",
                            rcv_process_thread_entry,
                            RT_NULL,
                            (rt_uint8_t *)&rcv_process_stack[0],
                            sizeof(rcv_process_stack),
                            10,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&rcv_process_thread);
    }
    return 0;
}
#define RT_USING_COMPONENTS_INIT
#define RT_SAMPLES_AUTORUN
#if defined (RT_SAMPLES_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
    INIT_APP_EXPORT(rcv_process_init);
#endif
