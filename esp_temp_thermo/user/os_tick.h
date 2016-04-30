#include "user_config.h"

#ifndef USER_OS_TICK_H_
#define USER_OS_TICK_H_

void tick_start(void);
void tick_stop(void);
uint32 tick_get(void);
void tick_sleep_ms(uint16 ms);
void tick_sleep_us(uint32 us);

#endif /* USER_OS_TICK_H_ */
