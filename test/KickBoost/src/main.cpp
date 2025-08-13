#include <Arduino.h>
#include "DigitalPinsSetup.h"

// DEBUG MODE
#define DEBUG_MODE 1        // 0 if not

#if DEBUG_MODE
	#define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // Nothing
#endif


// Interrupt
void IRAM_ATTR Button_ISR();            //
void IRAM_ATTR Control_ISR();           //

// Aux Functions
uint16_t Check_LEDs();                  // Read ADC and set LEDs

// Control
void BoostControl();

// Tasks
void PulseTask(void *params);       // Sets 100ms Pulse

// Handles and special variables
static TaskHandle_t PulseTaskHandle = NULL;     // Handle for the Notification
hw_timer_t* control_timer = NULL;               // Timer for the control interrupt [100us Ts]

// Global Variables
uint16_t Vo_12b     = 0;
uint16_t Duty_8b   = 0;
// float ek[2];
// float uk[2];


void setup() {
    Serial.begin(115200);
    
    // Setup [Digital Pins]
    LED_Setup();                // Set the 5 LEDs
    Pulse_Setup();              // Set the Pulse Digital out
    Button_Setup();             // Set the button Digital In

    // Setup [Other Pins]
    ADC_Setup();                // Set ADC pin [12bit res]    
    PWM_Setup();                // Set PWM     [80kHz, 8bit res]

    // Tasks and Interrupts
    // Timer_Setup();           // Control ISR Timer [runs every 100us]
    xTaskCreate(PulseTask, "PulseTask", 2048, NULL, 1, &PulseTaskHandle);
    attachInterrupt(BUTTON_PIN, Button_ISR, RISING);
}

void loop() {
	Check_LEDs();
    BoostControl();
    vTaskDelay(pdMS_TO_TICKS(50));
}


/* Pulse that Activates the solenoid, called from ISR*/
void PulseTask(void *params)
{
    uint16_t CapVoltage =0;
	while(1) 
  	{
    	// Waits until Hardware ISR awakens it
    	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Disables the Button Hardware interrupt
        detachInterrupt(BUTTON_PIN);

        // Make sure the LED's are if you kick
        CapVoltage = Check_LEDs();
        DEBUG_PRINT("ISR CALLED");

        // Only Kicks if its charged
        if(CapVoltage>= BOOST_22V_12B && CapVoltage < BOOST_26V_12B )
        {
            // Sets Pulse
    	    digitalWrite(PULSE_PIN, HIGH);
            DEBUG_PRINT("PULSE STARTED");
        
            // Only turns it off when the capacitor is below 10V
            while(CapVoltage > BOOST_10V_12B)
            {
                // Checks LED's very 10ms [Polling]
                CapVoltage = analogRead(ADC_PIN);
		        vTaskDelay(pdMS_TO_TICKS(10)); 
            }
            
            // Once the Voltage Drops below 10V, turns of pulse
    	    digitalWrite(PULSE_PIN, LOW);
        }
        
        // Waits 500ms [Deboucing reasons, the robot doesnt get the ball back that fast] 
		vTaskDelay(pdMS_TO_TICKS(500));
        
        // Re-enable button interrupt
        attachInterrupt(BUTTON_PIN, Button_ISR, RISING);
    }

}

/* Timer setup (for the Control interrupt) */
void Timer_Setup()
{
    control_timer = timerBegin(0, 80, true);                        // Prescaler = 80 => 1 tick = 1µs (80 MHz / 80)
    timerAttachInterrupt(control_timer, &Control_ISR, true);        // Attachs the Control Function and the timer
    timerAlarmWrite(control_timer, 1000, true);                      // 100 ticks -> 100µs
    timerAlarmEnable(control_timer);                                // Starts the timer
}


/* Kick Button ISR */ 
void IRAM_ATTR Button_ISR() {
    // Notify the pulse task from ISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(PulseTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(); // Yield if necessary
}

/* Control System ISR */
void IRAM_ATTR Control_ISR() {
    BoostControl();
}

void BoostControl()
{
    // Reads Boost Voltage
    Vo_12b = analogRead(ADC_PIN);

    // Apply Control Algorithm
    Duty_8b = ( Vo_12b>>4 ); // maps [0 - 2049] in [0 - 255]

    // Set PWM
    ledcWrite(PWM_CHANNEL, Duty_8b);

}


uint16_t Check_LEDs()
{
    // Read ADC
    uint16_t ADC12b = analogRead(ADC_PIN);

    DEBUG_PRINT("[BOOST CONVERTER]: Total Voltage: %.2f V\n",( ( (float)(ADC12b) )*ADC12B_TO_VOLT ));
    // DEBUG_PRINT("[BOOST CONVERTER]: Total 12b: %dV\n", ADC12b);

    // Set LED's based on Voltage
    digitalWrite(LED1_5V_PIN , ADC12b >= BOOST_5V_12B);
    digitalWrite(LED2_10V_PIN, ADC12b >= BOOST_10V_12B);
    digitalWrite(LED3_15V_PIN, ADC12b >= BOOST_15V_12B);
    digitalWrite(LED4_22V_PIN, ADC12b >= BOOST_22V_12B);
    digitalWrite(LED5_26V_PIN, ADC12b >= BOOST_26V_12B);

    return ADC12b;
}