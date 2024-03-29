/*
 * 程序清单：跑马灯例程
 *
 * 跑马灯大概是最简单的例子，就类似于每种编程语言中程序员接触的第一个程序
 * Hello World一样，所以这个例子就从跑马灯开始。创建一个线程，让它定时地对
 * LED进行更新（关或灭）
 */


#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx_hal_iwdg.h"
//#include "config.h"

#define RT_USING_SAMPLE_LED
#define LED_0_PIN 26
#define LED_1_PIN 59

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
/* 线程的TCB控制块 */
static struct rt_thread led_thread;


IWDG_InitTypeDef Idwg_init;
IWDG_HandleTypeDef Idwg_handle_type;
int idwg_init(void)
{
	Idwg_init.Prescaler=IWDG_PRESCALER_256;
	Idwg_init.Reload=625;
	Idwg_handle_type.Instance=IWDG;
	Idwg_handle_type.Init=Idwg_init;
	HAL_IWDG_Init(&Idwg_handle_type);
	return HAL_IWDG_Refresh(&Idwg_handle_type);
}
INIT_ENV_EXPORT(idwg_init);



void rt_hw_led_init(void)
{
    rt_pin_mode(LED_0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_1_PIN, PIN_MODE_OUTPUT);
}

void rt_hw_led_on(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        rt_pin_write(LED_0_PIN, 0x80);
        break;
    case 1:
        rt_pin_write(LED_1_PIN, 0x80);
        break;
    default:
        break;
    }
}

void rt_hw_led_off(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        rt_pin_write(LED_0_PIN, 0);
        break;
    case 1:
        rt_pin_write(LED_1_PIN, 0);
        break;
    default:
        break;
    }
}

static rt_uint8_t led_inited = 0;
void led(rt_uint32_t led, rt_uint32_t value)
{
    /* init led configuration if it's not inited. */
    if (!led_inited)
    {
        rt_hw_led_init();
        led_inited = 1;
    }

    if (led == 0)
    {
        /* set led status */
        switch (value)
        {
        case 0:
            rt_hw_led_off(0);
            break;
        case 1:
            rt_hw_led_on(0);
            break;
        default:
            break;
        }
    }

    if (led == 1)
    {
        /* set led status */
        switch (value)
        {
        case 0:
            rt_hw_led_off(1);
            break;
        case 1:
            rt_hw_led_on(1);
            break;
        default:
            break;
        }
    }
}

static void led_thread_entry(void *parameter)
{
    unsigned int count = 0;

    rt_hw_led_init();

    while (1)
    {
				HAL_IWDG_Refresh(&Idwg_handle_type);//feed dog in led.c
        /* led1 on */
        //rt_kprintf("led on, count : %d\r\n", count);
        count++;
        rt_hw_led_on(0);
				rt_hw_led_off(1);
        rt_thread_delay(RT_TICK_PER_SECOND / 2); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
        //rt_kprintf("led off\r\n");

        rt_hw_led_on(1);
			  rt_hw_led_off(0);
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}

int led_sample_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t *)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }
    return 0;
}
	/* 如果设置了RT_SAMPLES_AUTORUN，则加入到初始化线程中自动运行 */
#define RT_USING_COMPONENTS_INIT
#define RT_SAMPLES_AUTORUN
#if defined (RT_SAMPLES_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
    INIT_APP_EXPORT(led_sample_init);
#endif
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(led_sample_init, led sample);

