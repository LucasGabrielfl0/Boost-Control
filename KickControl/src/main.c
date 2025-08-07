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

// PWM
#define PWM_RES         1
#define PWM_FREQ        1

/* Global Variables */
// float uk[5];
// int ek[5];
// float yk[5];
// static uint32_t last_time = 0;


/* Function Prototypes */
// void PWM_Setup(int Freq_Hz);
// void Control_Setup(int CallbackFunc);
// void Control();
// void ActivateKick();        // Sends a 100ms Pulse that
void GPIO_Setup();

void app_main()
{
    /* ================================ SETUP ======================================== */
    GPIO_Setup();       // Setup for all LED's, the Button, and the Pulse output

    // control_Timer_Init();   // Setup Control Interrupt 
    // control_PWM_Init();     // Setup for the PWM 
    //                         // Setup for Kick Button Interrupt
    //                         // Setup for Kick Pulse

    // Read ADC
    uint16_t adc2 =1;
    float adc1 =(float)(adc2)*30/UINT16_MAX;

    // Set LED's based on Voltage
    gpio_set_level(LED1_5V_PIN, adc1 >= 5);
    gpio_set_level(LED2_10V_PIN, adc1 >= 10);
    gpio_set_level(LED3_15V_PIN, adc1 >= 15);
    gpio_set_level(LED4_22V_PIN, adc1 >= 22);
    gpio_set_level(LED5_26V_PIN, adc1 >= 26);

    // 10 ms delay between each read

}





void GPIO_Setup(){
    // Set LED pins as a GPIO
    gpio_pad_select_gpio(LED1_5V_PIN);
    gpio_pad_select_gpio(LED2_10V_PIN);
    gpio_pad_select_gpio(LED3_15V_PIN);
    gpio_pad_select_gpio(LED4_22V_PIN);
    gpio_pad_select_gpio(LED5_26V_PIN);

    // Set LED pins as an output
    gpio_set_direction(LED1_5V_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2_10V_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED3_15V_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED4_22V_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED5_26V_PIN, GPIO_MODE_OUTPUT);

    // Setup for the Button
    gpio_pad_select_gpio(INPUT_BUTTON_PIN);                         // Set as GPIO
    gpio_set_direction(INPUT_BUTTON_PIN, GPIO_MODE_INPUT);          // Set as output
    gpio_pulldown_en(INPUT_BUTTON_PIN);                             // Enable pull down
    gpio_pullup_dis(INPUT_BUTTON_PIN);                              // Disables Pull up, just for safety

    // Setup for the output Pulse [in practice, it's just another GPIO Output]
    gpio_pad_select_gpio(PULSE_PIN);
    gpio_set_direction(PULSE_PIN, GPIO_MODE_OUTPUT);
}
void Pulse_Setup(){




}




void Control_Hist(){
}

// /* Boost Circuit Control System: [every 1ms] */
// void Control(){
//     int ref;
//     int volt;
//     int Kp;
//     // Input Voltage
//     // read ADC

//     ek[0] = ref - volt;     // Calc Error   
//     yk[0] = Kp*ek[0];       // Apply Control

//     // Limits
//     if(yk[0] > 0.7)
//     {
//         yk[0] = 0.7;
//     }

//     if(yk[0] < 0.1)
//     {
//         yk[0] = 0.1;
//     }

//     // Update Variables
//     ek[1] = ek[0];
//     yk[1] = yk[0];

//     // uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
//     // DEBUG_PRINT("Time since last: %lu ms\n", now - last_time);
//     // last_time = now;

// }

// /* Hardware Interrupt: Button to activate Kick*/
// void Interrupt1(){
//     // Checks Voltage
//     float ADC_Volt = 1;
//     if( ADC_Volt >1 && ADC_Volt <26)
//     {
//         // Kicks
    
//     }
// }

// /* Sends a 100ms Pulse */
// void ActivateKick()
// {
//     // Send Signal
//     // Wait 100ms
// }




// void PWM_Dc(){

// }