#include "ControlTimer.h"

#define TIMER_DIVIDER         80
#define TIMER_INTERVAL_US     10000

#define TIMER_GROUP           TIMER_GROUP_0
#define TIMER_INDEX           TIMER_0

// static const char *TAG = "CTRL_TIMER";
// static volatile int adc_val = 0;

// 
static int64_t last_time = 0;

void IRAM_ATTR timer_isr(void *para) 
{
    // TIMERG0.int_clr_timers.t0 = 1;
    // TIMERG0.hw_timer[TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;


    // Time since last call
    int64_t now = esp_timer_get_time();
    int64_t delta = now - last_time;
    last_time = now;

    ESP_EARLY_LOGI("[Control]", " Δt: %lld us", delta);

}

// 
void control_timer_init(void) 
{
    // Timer config
    timer_config_t config = 
    {
        .divider = TIMER_DIVIDER,               // 80 MHz / TIMER_DIVIDER
        .counter_dir = TIMER_COUNT_UP,          // ...
        .counter_en = TIMER_PAUSE,              //
        .alarm_en = TIMER_ALARM_EN,             //
        .auto_reload = true                     // ...
    };

    // Timer Initialization
    timer_init              (TIMER_GROUP, TIMER_INDEX , &config);                   // Sets Config
    timer_set_counter_value (TIMER_GROUP, TIMER_INDEX , 0);                         // Start at 0
    timer_set_alarm_value   (TIMER_GROUP, TIMER_INDEX , TIMER_INTERVAL_US);         // Timer Period
    timer_enable_intr       (TIMER_GROUP, TIMER_INDEX );                            //  Enables Interrupt

    // Defines the callback function and starts the timer
    timer_isr_register (TIMER_GROUP, TIMER_INDEX , timer_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
    timer_start(TIMER_GROUP, TIMER_INDEX);
}
