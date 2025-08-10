#include <Arduino.h>
/*============================= LED's =======================*/
#define LED1_5V_PIN         GPIO_NUM_17     // Signals Capacitor Voltage > 5V
#define LED2_10V_PIN        GPIO_NUM_2      // Signals Capacitor Voltage > 10V
#define LED3_15V_PIN        GPIO_NUM_4      // Signals Capacitor Voltage > 15V
#define LED4_22V_PIN        GPIO_NUM_16     // Signals Capacitor Voltage > 22V
#define LED5_26V_PIN        GPIO_NUM_15     // Signals Capacitor Voltage > 26V

#define PULSE_PIN           GPIO_NUM_23     //
#define BUTTON_PIN          GPIO_NUM_5      //

#define ADC_PIN           	GPIO_NUM_34     //

#define PWM_PIN             GPIO_NUM_25     //
#define PWM_CHANNEL         0               //
#define PWM_FREQ            80000           // PWM frequency: 80 Khz
#define PWM_RES             8               // Resolution in bits, 12bits = [0 - 255]

// For Eficiency, generate every voltage Level as 12b number
// #define BOOST_5V_12B        (uint16_t)( (5.00f/30.0f) * 4095.0f )       
// #define BOOST_10V_12B       (uint16_t)( (10.0f/30.0f) * 4095.0f )
// #define BOOST_15V_12B       (uint16_t)( (15.0f/30.0f) * 4095.0f )
// #define BOOST_22V_12B       (uint16_t)( (22.0f/30.0f) * 4095.0f )
// #define BOOST_26V_12B       (uint16_t)( (26.0f/30.0f) * 4095.0f )

const uint16_t BOOST_5V_12B    =    (uint16_t)( (5.00f/30.0f) * 4095.0f );       
const uint16_t BOOST_10V_12B   =    (uint16_t)( (10.0f/30.0f) * 4095.0f );
const uint16_t BOOST_15V_12B   =    (uint16_t)( (15.0f/30.0f) * 4095.0f );
const uint16_t BOOST_22V_12B   =    (uint16_t)( (22.0f/30.0f) * 4095.0f );
const uint16_t BOOST_26V_12B   =    (uint16_t)( (26.0f/30.0f) * 4095.0f );


const float ADC12B_TO_VOLT =      (30.0f/4095.0f);


// DEBUG MODE
#define DEBUG_MODE 1        // 0 if not

#if DEBUG_MODE
	#define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // Nothing
#endif

// Setup Functions
void LED_Setup();
void ADC_Setup();
void Pulse_Setup();
void PWM_Setup();
void Timer_Setup();
void Button_Setup();

// Interrupt
void IRAM_ATTR Button_ISR();            //
void IRAM_ATTR Control_ISR();           //

// Aux Functions
uint16_t Check_LEDs();                 // Read ADC and set LEDs
float Check_LEDs_DEBUG();                 // Read ADC and set LEDs

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
    
    // Setup
    LED_Setup();
    Pulse_Setup();
    ADC_Setup();
    Button_Setup();
    
    PWM_Setup();
    // Timer_Setup();


    // Tasks and Interrupts
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


void LED_Setup()
{
  	pinMode(LED1_5V_PIN, OUTPUT);
  	pinMode(LED2_10V_PIN, OUTPUT);
  	pinMode(LED3_15V_PIN, OUTPUT);
  	pinMode(LED4_22V_PIN, OUTPUT);
  	pinMode(LED5_26V_PIN, OUTPUT);

	// Just for safety, turn it all off:
    digitalWrite(LED1_5V_PIN, LOW);
    digitalWrite(LED2_10V_PIN, LOW);
    digitalWrite(LED3_15V_PIN, LOW);
    digitalWrite(LED4_22V_PIN, LOW);
    digitalWrite(LED5_26V_PIN, LOW);

}

void Pulse_Setup()
{
  	pinMode(PULSE_PIN, OUTPUT);
  	digitalWrite(PULSE_PIN, LOW);
}

void Button_Setup()
{
  	pinMode(BUTTON_PIN, INPUT);
}

void ADC_Setup()
{
	pinMode(ADC_PIN, INPUT);

}
void PWM_Setup()
{
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
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


// float Check_LEDs_DEBUG()
// {
//     // Read ADC
//     uint16_t ADC12b = analogRead(ADC_PIN);
//     float ADC_Volt = ((float)(ADC12b))*ADC12B_TO_VOLT;

//     DEBUG_PRINT("[BOOST CONVERTER]: Total Voltage: %.2f V\n",ADC_Volt);
//     DEBUG_PRINT("[BOOST CONVERTER]: Total 12b: %dV\n",ADC12b);

//     // Set LED's based on Voltage
//     digitalWrite(LED1_5V_PIN , ADC_Volt >= 5.0);
//     digitalWrite(LED2_10V_PIN, ADC_Volt >= 10.0);
//     digitalWrite(LED3_15V_PIN, ADC_Volt >= 15.0);
//     digitalWrite(LED4_22V_PIN, ADC_Volt >= 22.0);
//     digitalWrite(LED5_26V_PIN, ADC_Volt >= 26.0);

//     return ADC_Volt;
// }


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