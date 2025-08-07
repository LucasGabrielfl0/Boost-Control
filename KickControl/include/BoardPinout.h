#ifndef _BOARD_PINOUT_H_
#define _BOARD_PINOUT_H_

/*============================= LED's =======================*/
#define LED1_5V_PIN         GPIO_NUM_17     // Signals Capacitor Voltage > 5V
#define LED2_10V_PIN        GPIO_NUM_2      // Signals Capacitor Voltage > 10V
#define LED3_15V_PIN        GPIO_NUM_4      // Signals Capacitor Voltage > 15V
#define LED4_22V_PIN        GPIO_NUM_16     // Signals Capacitor Voltage > 22V
#define LED5_26V_PIN        GPIO_NUM_15     // Signals Capacitor Voltage > 26V


/*============================== Control Pins ============================*/
#define INPUT_BUTTON_PIN    GPIO_NUM_5      // Input button, User activates kick
#define PULSE_PIN           GPIO_NUM_23     // Digital out Pin: 100 ms pulse actives Kick
// #define PWM_PIN             GPIO_NUM_23     // PWM Pin: Boost Converter Voltage Control



#endif