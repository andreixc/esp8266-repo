#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "uart.h"
#include <ip_addr.h>
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "os_tick.h"
#include "dht11_sensor.h"

#define WAIT_FOR_SOF	0
#define READING_DATA	1
#define DHT_DONE		3

static volatile uint8 dht_temperature = 0, dht_humidity = 0;
static uint32 _timestamp = 0;

static uint8 _status;
static uint8 data_bits[5], data_index;

static os_timer_t dht_timer;

LOCAL void print_console(char * arg)
{
	uart0_tx_buffer(arg,strlen(arg));
}

static void gpio_intr_handler(void * arg)
{
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	uint32 state = GPIO_INPUT_GET(GPIO_0);
	if(gpio_status & BIT(0))
	{
		uint8 byte, bit;
		gpio_pin_intr_state_set(GPIO_ID_PIN(0),GPIO_PIN_INTR_DISABLE);

		if(_timestamp == 0)
		{
			if(state == HIGH)
				_timestamp = tick_get();
		}
		else
		{
			if(state == LOW)
			{
				_timestamp = tick_get() - _timestamp;
				switch(_status)
				{
				case WAIT_FOR_SOF:
					if(_timestamp > 80)
					{
						_status = READING_DATA;
						data_index = 0;
					}
						break;

				case READING_DATA:
					data_bits[data_index>>3] <<= 1;
					if(_timestamp > 60 && _timestamp < 80)
						data_bits[data_index>>3] |= 1;

					data_index++;
					if(data_index == 40)
					{
						/* validate the checksum */
						if(data_bits[4] == (data_bits[0] + data_bits[1] + data_bits[2] + data_bits[3]))
						{
							dht_temperature = data_bits[2];
							dht_humidity = data_bits[0];
						}
						_status = DHT_DONE;
					}
					break;
				default:
					break;

				}
				_timestamp = 0; // reset
			}
		}
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(0));
		gpio_pin_intr_state_set(GPIO_ID_PIN(0),GPIO_PIN_INTR_ANYEDGE);
	}
}

static void os_ticker_irq(void *arg)
{
	/* reset all timestamps */
	_status = WAIT_FOR_SOF;
	_timestamp = 0;
	os_memset(data_bits,0,sizeof(data_bits));

	// send start of frame
	GPIO_OUTPUT_SET(GPIO_0,LOW);
	os_delay_us(20000);
	GPIO_OUTPUT_SET(GPIO_0,HIGH);

	// set as input
	GPIO_DIS_OUTPUT(GPIO_0);
	// enable gpio irq
	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(gpio_intr_handler,NULL);
	gpio_register_set(GPIO_PIN_ADDR(GPIO_0),
			 GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)  |
			 GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE) |
			 GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE)
	);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));
	gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_ANYEDGE);
	ETS_GPIO_INTR_ENABLE();
}

void dht11_setup(void)
{
	/* disarm local timer */
	os_timer_disarm(&dht_timer);
    /* Set callback function */
    os_timer_setfn(&dht_timer, (os_timer_func_t *)os_ticker_irq, NULL);

    /* setup the GPIO0 as data pin for DHT11 */
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	//PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO0_U);
	//PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);
	GPIO_OUTPUT_SET(GPIO_0,HIGH); /* set bus to idle - by default */

    /* arm timer for 3s irq */
    os_timer_arm(&dht_timer,3000, 1);
  }

void dht11_stop(void)
{
	os_timer_disarm(&dht_timer);
}

void dht11_read(uint8 * temp, uint8 * humidity)
{
	*temp = dht_temperature;
	*humidity = dht_humidity;
}
