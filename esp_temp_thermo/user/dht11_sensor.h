/*
 * dht11_sensor.h
 *
 *  Created on: 8 Nov 2015
 *      Author: eraser
 */

#ifndef USER_DHT11_SENSOR_H_
#define USER_DHT11_SENSOR_H_

void dht11_setup(void);
void dht11_stop(void);
void dht11_read(uint8 * temp, uint8 * humidity);


#define GPIO_0		(0)
#define GPIO_2		(2)
#define LOW			(0)
#define HIGH		(1)

#endif /* USER_DHT11_SENSOR_H_ */
