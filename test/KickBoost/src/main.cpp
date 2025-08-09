#include <Arduino.h>
/*============================= LED's =======================*/
#define LED1_5V_PIN         GPIO_NUM_17     // Signals Capacitor Voltage > 5V
#define LED2_10V_PIN        GPIO_NUM_2      // Signals Capacitor Voltage > 10V
#define LED3_15V_PIN        GPIO_NUM_4      // Signals Capacitor Voltage > 15V
#define LED4_22V_PIN        GPIO_NUM_16     // Signals Capacitor Voltage > 22V
#define LED5_26V_PIN        GPIO_NUM_15     // Signals Capacitor Voltage > 26V

#define PULSE_PIN           GPIO_NUM_23     //
#define PWM_PIN             GPIO_NUM_35     //
#define BUTTON_PIN          GPIO_NUM_5      //

#define ADC_PIN           	GPIO_NUM_34     //

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
void Button_Setup();

// Interrupt
void IRAM_ATTR Button_ISR();        //

// Aux Functions
float Check_LEDs();                 // Read ADC and set LEDs
void PulseTask(void *params);       // Sets 100ms Pulse

// Global Variables
static TaskHandle_t PulseTaskHandle = NULL;     // Handle for the Notification

void setup() {
    Serial.begin(115200);
    
    // Setup
    LED_Setup();
    Pulse_Setup();
    ADC_Setup();
    Button_Setup();

    // Tasks and Interrupts
    xTaskCreate(PulseTask, "PulseTask", 2048, NULL, 1, &PulseTaskHandle);
    attachInterrupt(BUTTON_PIN, Button_ISR, RISING);
}

void loop() {
	Check_LEDs();
	vTaskDelay(pdMS_TO_TICKS(50));
}

void PulseTask(void *params)
{
    float CapVoltage =0;
	while(1) 
  	{
    	// Waits until Hardware ISR awakens it
    	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Disables interrupt
        detachInterrupt(BUTTON_PIN);

        CapVoltage = Check_LEDs();
        DEBUG_PRINT("ISR CALLED");

        // Only sets if its charged
        if(CapVoltage>= 22 && CapVoltage < 26 )
        {
            DEBUG_PRINT("PULSE STARTED");
    	    digitalWrite(PULSE_PIN, HIGH);        // Turn on
		    vTaskDelay(pdMS_TO_TICKS(100));       // Waits 100ms 
    	    digitalWrite(PULSE_PIN, LOW);         // Turns off
		    vTaskDelay(pdMS_TO_TICKS(500));       // Waits 500ms [Deboucing reasons, the robot doesnt get the ball back that fast] 
        }

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




// Kick Button ISR
void IRAM_ATTR Button_ISR() {
    // Notify the pulse task from ISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(PulseTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(); // Yield if necessary
}




float Check_LEDs()
{
    // Read ADC
    uint16_t ADC12b = analogRead(ADC_PIN);
    float ADC_Volt = ((float)(ADC12b))*ADC12B_TO_VOLT;

    DEBUG_PRINT("[BOOST CONVERTER]: Total Voltage: %.2f V\n",ADC_Volt);
    DEBUG_PRINT("[BOOST CONVERTER]: Total 12b: %dV\n",ADC12b);

    // Set LED's based on Voltage
    digitalWrite(LED1_5V_PIN , ADC_Volt >= 5.0);
    digitalWrite(LED2_10V_PIN, ADC_Volt >= 10.0);
    digitalWrite(LED3_15V_PIN, ADC_Volt >= 15.0);
    digitalWrite(LED4_22V_PIN, ADC_Volt >= 22.0);
    digitalWrite(LED5_26V_PIN, ADC_Volt >= 26.0);

    return ADC_Volt;
}