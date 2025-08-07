#ifndef _CONTROL_TIMER_H_
#define _CONTROL_TIMER_H_

#include "driver/timer.h"
#include "driver/adc.h"
#include "esp_intr_alloc.h"

void Control_Timer_Init();

void IRAM_ATTR timer_isr(void *para);
void control_timer_init(void);

#endif