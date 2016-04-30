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


void tick_start(void)
{
	return;
}

void tick_stop(void)
{
	return;
}

uint32 tick_get(void)
{
	return system_get_time();
}

void tick_sleep_us(uint32 us)
{
	os_delay_us(us);
}

void tick_sleep_ms(uint16 ms)
{
	tick_sleep_ms((uint32)ms*1000);
}
