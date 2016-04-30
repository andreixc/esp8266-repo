/*
 * wifi_client.h
 *
 *  Created on: 9 Nov 2015
 *      Author: eraser
 */

#ifndef USER_WIFI_CLIENT_H_
#define USER_WIFI_CLIENT_H_

typedef enum
{
	WIFI_IP_ADDRESS,
	WIFI_CONNECTED,
	WIFI_HOSTNAME,
	WIFI_SSID,
	WIFI_PASSWORD

}WIFI_PARAMETER;

void wifi_start(void);
void wifi_stop(void);
void wifi_set_parameter(WIFI_PARAMETER param, void * value);
void wifi_get_parameter(WIFI_PARAMETER param, void * value);


#endif /* USER_WIFI_CLIENT_H_ */
