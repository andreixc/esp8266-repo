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
#include "wifi_client.h"
#include "os_tick.h"
#include "config.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
struct scan_config config_scanner;

static void user_procTask(os_event_t *events);

LOCAL void print_console(char * arg)
{
	uart0_tx_buffer(arg,strlen(arg));
}

void print_data(uint8 temp, uint8 hum)
{
	char data[30]={0};
	os_printf("T=%d\tHU=%d%%\r\n",temp,hum);
	print_console(data);
}

void delay_ms(uint16 ms)
{
	while(ms > 0)
	{
		ms--;
		os_delay_us(1000);
	}

}

//Do nothing function
static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
	os_delay_us(1000);
}

LOCAL void ICACHE_FLASH_ATTR _scanDone(void* result, STATUS status)
{

	if(status == OK)
	{

		struct bss_info * it_wifi;
		print_console("SCAN OK\r\n\r\n");
		int index = 0;
		for(it_wifi = STAILQ_NEXT((struct bss_info *)result,next); it_wifi; it_wifi = STAILQ_NEXT(it_wifi,next))
		{
			char name[70];
			uint8_t qos;

			os_sprintf(name,"SSID : %s\r\n",it_wifi->ssid);
			print_console(name);

			os_sprintf(name,"BSSID : %2X:%2X:%2X:%2X:%2X:%2X\r\n",
					it_wifi->bssid[0],it_wifi->bssid[1],it_wifi->bssid[2],
					it_wifi->bssid[3],it_wifi->bssid[4],it_wifi->bssid[5]
			);
			print_console(name);

			os_sprintf(name,"CH : %d\r\n",it_wifi->channel);
			print_console(name);

			os_sprintf(name,"RSSI : %d\r\n",it_wifi->rssi);
			print_console(name);

			// calculate quality //
			if(it_wifi->rssi <= -100)
			{
				qos = 0;
			}
			else if(it_wifi->rssi >= -50)
			{
				qos = 100;
			}
			else
			{
				qos = 2*(it_wifi->rssi + 100);
			}

			os_sprintf(name,"QOS : %d %%\r\n",qos);
			print_console(name);

			//

			os_sprintf(name,"AUTH : %d\r\n",
					it_wifi->authmode);
			print_console(name);

			print_console("\r\n");

		}
	}
	else
		print_console("SCAN ERROR\r\n");
}



LOCAL void ICACHE_FLASH_ATTR
station_wifi_init(void)
{

	if(wifi_get_opmode() != STATION_MODE)
	{
		wifi_set_opmode(STATION_MODE);
	}

	//
	if(wifi_get_opmode() != STATION_MODE)
	{
		// fail and get out
		print_console("Failed to go into STATION_MODE\r\n");
		return;
	}

	os_memset(&config_scanner,0,sizeof(config_scanner));
	wifi_station_scan(&config_scanner,&_scanDone);
}

static os_timer_t xticker;
static void xticker_irq(void * arg)
{
	uint8 temp, hum;
	dht11_read(&temp,&hum);
	print_data(temp,hum);
}

LOCAL STATUS ICACHE_FLASH_ATTR uart0_one_char(uint8 txChr)
{
	if(txChr == '\n')
	{
		txChr = '\r';
		uart0_tx_buffer(&txChr,1);
		txChr = '\n';
	}
	uart0_tx_buffer(&txChr,1);
	return OK;
}

/* Applicaton entry point */
void ICACHE_FLASH_ATTR
user_init()
{

	/* timer reinit */
	system_timer_reinit();
	/* Initialize the UART subsystem */
    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    os_install_putc1(uart0_one_char);

    /* setup layers */
    tick_start();
    dht11_setup();
    //relay_init();

	/* Disarm ticker */
    os_timer_disarm(&xticker);
    /* Set callback function */
    os_timer_setfn(&xticker, (os_timer_func_t *)xticker_irq, NULL);
    /* arm timer for 10us irq */
    os_timer_arm(&xticker, 3000, 1);


    //wifi_set_sleep_type
    wifi_set_parameter(WIFI_SSID,(void *)WIFI_AP);
    wifi_set_parameter(WIFI_PASSWORD,(void *)WIFI_PASS);
    wifi_start();

    /* work the wifi system */
    //station_wifi_init();

    /* Start os task */
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}
