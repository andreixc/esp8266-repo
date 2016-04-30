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
#include "relay.h"

void relay_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	GPIO_OUTPUT_SET(GPIO_2,LOW);
}

void relay_set(uint8 on_off)
{
	GPIO_OUTPUT_SET(GPIO_2,
			(on_off == 0) ? LOW : HIGH);
}

uint8 relay_get(void)
{
	return GPIO_INPUT_GET(GPIO_2);
}
