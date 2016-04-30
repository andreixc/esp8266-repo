/*
 * relay.h
 *
 *  Created on: 19 Nov 2015
 *      Author: eraser
 */

#include "user_config.h"
#ifndef USER_RELAY_H_
#define USER_RELAY_H_

void relay_init(void);
void relay_set(uint8);
uint8 relay_get(void);

#endif /* USER_RELAY_H_ */
