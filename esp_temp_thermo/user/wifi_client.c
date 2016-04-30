#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "ip_addr.h"
#include "user_config.h"
#include "uart.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "os_tick.h"
#include "wifi_client.h"
#include "espconn.h"
#include "relay.h"

typedef struct
{
	uint8 ipaddr[4];
	uint8 gateway[4];
	uint8 netmask[4];
	uint8 dns1[4], dns2[4];
	uint8 connected;
	char ssid[32];
	char password[64];
	char hostname[32];

}LOCAL_PARAMETERS;

static LOCAL_PARAMETERS wifi_cofiguration={0};
static struct station_config esp_config;

/* TCP */
static struct espconn conn1;
static esp_tcp tcp1;

static void connectCB(void *arg) {
	char data[40];
	uint8 temp, hum;

	struct espconn *pConn = (struct espconn *)arg;
	// By default, an ESP TCP connection times out after 10 seconds
	// Change to the max value.
	/*espconn_regist_time(pConn, 7200, 1);
	os_printf("Connect cb!!\n");
	dht11_read(&temp,&hum);
	os_sprintf(data,"T=%d,HUM=%d\r\n",temp,hum);
	espconn_sent(pConn,data,strlen(data));
	espconn_disconnect(pConn);
	os_printf("Ending connection...\n");
	//pTelnetClientConn = pConn;
	 *
	 */
	//relay_set(TRUE);
}

static void receiveCB(void *arg, char *pData, unsigned short len) {
	os_printf("Receive cb!!  len=%d\n", len);
	struct espconn *pConn = (struct espconn *)arg;
	char response[100] = {0};
	char multi_string[50] = {0};
	char * response_header =
			"HTTP 1.0 200 OK\r\nConnection: close\r\n\r\n"
			"<html><head><title> Relay controller</title></head>";

			;
	memcpy(response,response_header,strlen(response_header));
	if(os_memcmp("GET",pData,3) == 0)
	{
		if(os_strstr(pData,"/open"))
		{
			relay_set(TRUE);
			os_sprintf(multi_string,"<body><p>Status : %s</p></body></html>\r\n",relay_get() ? "ON" : "OFF");
		}
		else if(os_strstr(pData,"/close"))
		{
			relay_set(FALSE);
			os_sprintf(multi_string,"<body><p>Status : %s</p></body></html>\r\n",relay_get() ? "ON" : "OFF");
		}
		else if(os_strstr(pData,"/temp"))
		{
			uint8 temp, hum;
			dht11_read(&temp,&hum);
			os_sprintf(multi_string,"<body><p>Temperature : %d C</p></body></html>\r\n",temp);
		}
		else if(os_strstr(pData,"/hum"))
		{
			uint8 temp, hum;
			dht11_read(&temp,&hum);
			os_sprintf(multi_string,"<body><p>Humidity : %d </p></body></html>\r\n",hum);
		}
		else if(os_strstr(pData,"/sleep"))
		{
			system_deep_sleep(123000000);
		}
		else
		{
			os_sprintf(multi_string,"<body>Humidity/Temperature/Relay server</body></html>\r\n");
		}

		os_strcat(response,multi_string);
		espconn_send(pConn,response,strlen(response)+1);
	}
	espconn_disconnect(pConn);
}

static void disconnectCB(void *arg) {
	//relay_set(FALSE);
	os_printf("Disconnect cb!!\n");
}

void ICACHE_FLASH_ATTR esp_wifi_callback( System_Event_t * evt )
{
	struct mdns_info * info_data;
	os_printf("Event: %d\n",evt->event);
	switch(evt->event)
	{
		case EVENT_STAMODE_CONNECTED:
			break;

		case EVENT_STAMODE_DISCONNECTED:
			break;

		case EVENT_STAMODE_AUTHMODE_CHANGE:
			break;

		case EVENT_STAMODE_GOT_IP:
			/* got ip */
			os_printf("EVENT_STAMODE_GOT_IP\n");
			os_printf("STARTING TCP SERVER...\n");

			tcp1.local_port = 80;
			conn1.type = ESPCONN_TCP;
			conn1.state = ESPCONN_NONE;
			conn1.proto.tcp = &tcp1;
			espconn_regist_connectcb(&conn1, connectCB);
			espconn_regist_disconcb(&conn1, disconnectCB);
			espconn_regist_recvcb(&conn1, receiveCB);
			espconn_accept(&conn1);
			/*info_data = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
			info_data->host_name = "esp_service";
			info_data->ipAddr = evt->event_info.got_ip.ip.addr;
			info_data->server_name = "temp_humidity";
			info_data->server_port = 8080;
			info_data->txt_data[0] = "version = now";
			espconn_mdns_init(info_data);*/
			break;

		case EVENT_SOFTAPMODE_STACONNECTED:
			break;

		case EVENT_SOFTAPMODE_STADISCONNECTED:
			break;

	}
}

void ICACHE_FLASH_ATTR wifi_start(void)
{
	os_printf("wifi_start\r\n");
	wifi_station_set_hostname(wifi_cofiguration.hostname);
	wifi_set_opmode_current( STATION_MODE );
	esp_config.bssid_set = 0;
	os_memcpy( &esp_config.ssid ,wifi_cofiguration.ssid,32);
	os_memcpy( &esp_config.password,wifi_cofiguration.password,64);
	wifi_station_set_config( &esp_config );
	wifi_set_event_handler_cb(esp_wifi_callback);
}

void wifi_stop(void)
{

}

void wifi_set_parameter(WIFI_PARAMETER param, void * value)
{
	switch(param)
	{
		case WIFI_IP_ADDRESS:
			break;
		case WIFI_HOSTNAME:
			os_memcpy(wifi_cofiguration.hostname, value,sizeof(wifi_cofiguration.hostname));
			break;
		case WIFI_SSID:
			os_memcpy(wifi_cofiguration.ssid, value,strlen(value) + 1);
			break;
		case WIFI_PASSWORD:
			os_memcpy(wifi_cofiguration.password, value,strlen(value) + 1);
			break;
	}
}

void wifi_get_parameter(WIFI_PARAMETER param, void * value)
{
	switch(param)
	{
		case WIFI_IP_ADDRESS:
			break;
		case WIFI_HOSTNAME:
			//os_memcpy(wifi_cofiguration.hostname, value,sizeof(wifi_cofiguration.hostname));
			break;
		case WIFI_SSID:
			//os_memcpy(wifi_cofiguration.ssid, value,strlen(value) + 1);
			break;
		case WIFI_PASSWORD:
			//os_memcpy(wifi_cofiguration.password, value,strlen(value) + 1);
			break;
	}
}

