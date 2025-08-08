/* Kick Control: 
* 1.1 
* 1.2
* 1.3 
* 
* 
* 
*/
#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"            // ESP32 GPIO Functions
#include "driver/adc.h"             // ESP32 ADC Functions
#include "driver/ledc.h"            // ESP32 PWM Functons

#include "freertos/FreeRTOS.h"      // Hal delay etc
#include "freertos/task.h"
#include "freertos/queue.h"

// #include "Interrupt_Timer.h"
// #include "PWM_Timer.h"
#include "BoardPinout.h"            // Custom Board Pinnout

// DEBUG MODE
#define DEBUG_MODE 1        // 0 if not

#if DEBUG_MODE
    #define DEBUG_PRINT(fmt, ...) ESP_LOGI("DEBUG", fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // Nothing
#endif


// #define CONTROL_TASK(fmt, ...) ESP_LOGI("DEBUG", fmt, ##__VA_ARGS__)


// PWM
// #define PWM_RES         1
// #define PWM_FREQ        1



/* Function Prototypes */
// SETUP FUNCTIONS
// void PWM_Setup(int Freq_Hz);
// void Control_Setup(int CallbackFunc);
void KickButton_Setup();        // Sends a 100ms Pulse that
void GPIO_Setup();
void ADC_Setup();

// Tasks
void SetPulse_Task(void *params);


// Others
float Check_LEDs();



// static void IRAM_ATTR isr_Button(void *args)
// {

//     // Flags the Pulse function

// }





void app_main()
{
    /* ================================ SETUP ======================================== */
    // Buttons / LED's
    GPIO_Setup();               // Setup for all LED's, and the Pulse output
    // KickButton_Setup();         // Setup for the Button input thats indicates you need to kick
    
    // Timers and PWM

    // control_Timer_Init();   // Setup Control Interrupt 
    // control_PWM_Init();     // Setup for the PWM 
    //                         // Setup for Kick Pulse

    // ADC
    ADC_Setup();

    // Tasks
    // xTaskCreate(SetPulse_Task, "SetPulse", 2048, NULL, 1 , NULL);
    // xTaskCreate(Hyst_Control_task, "ControlTask", 2048, NULL, 1 , NULL);

    // Interrupts

    float AdcVolt = 0;
    /* ================================ MAIN LOOP ======================================== */
    while(1)
    {
        // Gets Boost voltage and sets LEDs
        AdcVolt = Check_LEDs();

    }

    // 10 ms delay between each read
}


void ADC_Setup()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_PIN, ADC_ATTEN_DB_11);
}






// void KickButton_Setup()
// {
//     // Setup for the Button
//     gpio_pad_select_gpio(INPUT_BUTTON_PIN);                         // Set as GPIO
//     gpio_set_direction(INPUT_BUTTON_PIN, GPIO_MODE_INPUT);          // Set as output
//     gpio_pulldown_en(INPUT_BUTTON_PIN);                             // Enable pull down
//     gpio_pullup_dis(INPUT_BUTTON_PIN);                              // Disables Pull up, just for safety
// gpio_pad_select_gpio()
//     // Setup for Kick Button Interrupt
//     // gpio_set_intr_type(INPUT_BUTTON_PIN, GPIO_INTR_POSEDGE);        // Interrupt in the rising edge
//     // gpio_isr_register
//     // gpio_install_isr_service(0);
//     // gpio_isr_handler_add(INPUT_BUTTON_PIN, isr_Button, (void*) INPUT_BUTTON_PIN);
// }



float Check_LEDs()
{
    // Read ADC
    uint16_t ADC16b = adc1_get_raw(ADC_PIN);
    float ADC_Volt =(float)(ADC16b)*ADC12B_TO_VOLT;

    DEBUG_PRINT("[BOOST CONVERTER]: Total Voltage: %.2f V\n",ADC_Volt);

    // Set LED's based on Voltage
    gpio_set_level(LED1_5V_PIN , ADC_Volt >= 5);
    gpio_set_level(LED2_10V_PIN, ADC_Volt >= 10);
    gpio_set_level(LED3_15V_PIN, ADC_Volt >= 15);
    gpio_set_level(LED4_22V_PIN, ADC_Volt >= 22);
    gpio_set_level(LED5_26V_PIN, ADC_Volt >= 26);

    return ADC_Volt;
}

void SetPulse_Task(void *params){
    float AdcVolt = 0;

    while(1)
    {
        // Only runs when the Hardware ISR gets called
        if(1)
        {
            AdcVolt = Check_LEDs();
            if(AdcVolt >= 22 && AdcVolt < 26)
            {
                // Disables Interrupt from the pin
                // gpio_isr_handler_remove(INPUT_BUTTON_PIN);

                // Turns on the pulse
                gpio_set_level(LED1_5V_PIN, 1);  

                // waits 100ms   
                vTaskDelay(100/portTICK_PERIOD_MS);

                // Turns off the pulse
                gpio_set_level(LED1_5V_PIN, 0);
                
                // Enables ISR the be called again
                // gpio_isr_handler_add(INPUT_BUTTON_PIN, isr_Button, (void*) INPUT_BUTTON_PIN);

            }

        }
       
    }

}


void PI_Control_task(){
}

void Hyst_Control_task(){
}
