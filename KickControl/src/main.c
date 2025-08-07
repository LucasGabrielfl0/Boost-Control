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
#include "driver/adc.h"

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
void KickButton_Setup();        // Sends a 100ms Pulse that
void GPIO_Setup();
void SetPulse_Task(void *params);

static void IRAM_ATTR isr_Button(void *args)
{

    // Flags the Pulse function

}


void ADC_Setup();

void app_main()
{
    /* ================================ SETUP ======================================== */
    GPIO_Setup();               // Setup for all LED's, and the Pulse output
    KickButton_Setup();         // Setup for the Button input thats indicates you need to kick
    
    // control_Timer_Init();   // Setup Control Interrupt 
    // control_PWM_Init();     // Setup for the PWM 
    //                         // Setup for Kick Button Interrupt
    //                         // Setup for Kick Pulse

    // Tasks
    xTaskCreate(SetPulse_Task, "SetPulse", 2048, NULL, 1 , NULL);
    // xTaskCreate(Hyst_Control_task, "ControlTask", 2048, NULL, 1 , NULL);

    // Interrupts


    /* ================================ MAIN LOOP ======================================== */

    // Read ADC
    uint16_t ADC16b =1;
    float ADC_Volt =(float)(ADC16b)*ADC12B_TO_VOLT;

    // Set LED's based on Voltage
    gpio_set_level(LED1_5V_PIN, ADC_Volt >= 5);
    gpio_set_level(LED2_10V_PIN, ADC_Volt >= 10);
    gpio_set_level(LED3_15V_PIN, ADC_Volt >= 15);
    gpio_set_level(LED4_22V_PIN, ADC_Volt >= 22);
    gpio_set_level(LED5_26V_PIN, ADC_Volt >= 26);

    // 10 ms delay between each read

}
void fun2();
void ADC_Setup()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    // adc1_config_channel_atten(ADC_PIN, ADC_);



}



void KickButton_Setup()
{
    // Setup for the Button
    gpio_pad_select_gpio(INPUT_BUTTON_PIN);                         // Set as GPIO
    gpio_set_direction(INPUT_BUTTON_PIN, GPIO_MODE_INPUT);          // Set as output
    gpio_pulldown_en(INPUT_BUTTON_PIN);                             // Enable pull down
    gpio_pullup_dis(INPUT_BUTTON_PIN);                              // Disables Pull up, just for safety
    gpio_set_intr_type(INPUT_BUTTON_PIN, GPIO_INTR_POSEDGE);        // Interrupt in the rising edge

    // gpio_isr_register
    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_BUTTON_PIN, fun2, (void*) INPUT_BUTTON_PIN);
}
void fun2();




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

    // Setup for the output Pulse [in practice, it's just another GPIO Output]
    gpio_pad_select_gpio(PULSE_PIN);
    gpio_set_direction(PULSE_PIN, GPIO_MODE_OUTPUT);
}

uint16_t SetLEDs();
uint16_t SetLEDs()
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

    if(ADC_Volt >= 22 && ADC_Volt < 26)
    {
        return 1;
    }
    else
    {
       return 0;
    }
}

void SetPulse_Task(void *params){
    uint16_t Adc_16b = 0;

    while(1)
    {
        // Only runs when the Hardware ISR gets called
        if(1 && SetLEDs() == 1)
        {
            // Disables Interrupt from the pin
            gpio_isr_handler_remove(INPUT_BUTTON_PIN);

            // Turns on the pulse
            gpio_set_level(LED1_5V_PIN, 1);  

            // waits 100ms   
            vTaskDelay(100/portTICK_PERIOD_MS);

            // Turns off the pulse
            gpio_set_level(LED1_5V_PIN, 0);
            
            // Enables ISR the be called again
            gpio_isr_handler_add(INPUT_BUTTON_PIN, fun2, (void*) INPUT_BUTTON_PIN);
        }
       
    }

}


void PI_Control_task(){
}

void Hyst_Control_task(){
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


// /* Sends a 100ms Pulse */
// void ActivateKick()
// {
//     // Send Signal
//     // Wait 100ms
// }




// void DC_to_PWM(){

// }