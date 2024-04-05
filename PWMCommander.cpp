// PWMCommander.cpp


#define _DCCSimpleWebServer_ 1

#include "Arduino.h"

#include "PWMCommander.h"


#define PARAMNAME_CHANNEL "ch"
#define PARAMNAME_DCCVALUE "dcc"



// 4+1 bit control

#define PWM_RAIL1_OUT_PIN  18
#define PWM_RAIL2_OUT_PIN   5
#define PWM_RAIL3_OUT_PIN  17
#define PWM_RAIL4_OUT_PIN  16

#define PWM_RAILEN_OUT_PIN  4

#define PWM_PERIOD          100


hw_timer_t* timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE webCommandMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t isrLoop = 0;


volatile bool isrPWMdirectionNext;
volatile bool isrPWMdirectionNow;
volatile uint16_t isrPWMvalueNext;  // dutyCycle
volatile uint16_t isrPWMvalueNow;
volatile bool isrPWMplus;
volatile bool isrDirectionChange;
volatile uint32_t isrTimerAlarmValue;

volatile uint64_t isrBootTime; // in uSec from boot time


void IRAM_ATTR onTimer() {

    digitalWrite(PWM_RAILEN_OUT_PIN, false);

    portENTER_CRITICAL_ISR(&timerMux);
    isrCounter++;

    isrBootTime = esp_timer_get_time();

    portEXIT_CRITICAL_ISR(&timerMux);

    isrDirectionChange = false;
    if (isrPWMplus) {
        if (isrPWMdirectionNext != isrPWMdirectionNow)
            isrDirectionChange = true;
        isrPWMdirectionNow = isrPWMdirectionNext;
        if (isrPWMvalueNext != isrPWMvalueNow) {
            isrPWMvalueNow = isrPWMvalueNext;
            if (isrPWMvalueNow < 0)
                isrPWMvalueNow = 0;
            if( isrPWMvalueNow >= PWM_PERIOD)
                isrPWMvalueNow = PWM_PERIOD -1;
        }
    }

    isrPWMplus = !isrPWMplus;
 
    isrLoop++;
    if (isrLoop == 120000)
        isrLoop = 0;
    if (isrLoop == 0)
        xSemaphoreGiveFromISR(timerSemaphore, NULL);

    if (!isrPWMplus) {
            digitalWrite(PWM_RAIL2_OUT_PIN, false);
            digitalWrite(PWM_RAIL3_OUT_PIN, false);
            digitalWrite(PWM_RAIL1_OUT_PIN, false);
            digitalWrite(PWM_RAIL4_OUT_PIN, false);
            if (isrPWMvalueNow >= 0 && isrPWMvalueNow < PWM_PERIOD)
                isrTimerAlarmValue = PWM_PERIOD - isrPWMvalueNow;
            else
                isrTimerAlarmValue = PWM_PERIOD;
    }
    else {
        if (isrPWMdirectionNow) {
            digitalWrite(PWM_RAIL1_OUT_PIN, false);
            digitalWrite(PWM_RAIL4_OUT_PIN, false);
            digitalWrite(PWM_RAIL2_OUT_PIN, true);
            digitalWrite(PWM_RAIL3_OUT_PIN, true);
            if (isrPWMvalueNow >= 0 && isrPWMvalueNow < PWM_PERIOD)
                isrTimerAlarmValue = isrPWMvalueNow;
        }
        else {
            digitalWrite(PWM_RAIL2_OUT_PIN, false);
            digitalWrite(PWM_RAIL3_OUT_PIN, false);
            digitalWrite(PWM_RAIL1_OUT_PIN, true);
            digitalWrite(PWM_RAIL4_OUT_PIN, true);
            if (isrPWMvalueNow >= 0 && isrPWMvalueNow < PWM_PERIOD)
                isrTimerAlarmValue = isrPWMvalueNow;
        }
    }
    timerAlarmWrite(timer, isrTimerAlarmValue, true);

    if (isrPWMvalueNow > 0)
        digitalWrite(PWM_RAILEN_OUT_PIN, true);
}


void SetupPWMCommander()
{

    pinMode(PWM_RAIL1_OUT_PIN, OUTPUT);
    digitalWrite(PWM_RAIL1_OUT_PIN, false);
    pinMode(PWM_RAIL2_OUT_PIN, OUTPUT);
    digitalWrite(PWM_RAIL2_OUT_PIN, false);
    pinMode(PWM_RAIL3_OUT_PIN, OUTPUT);
    digitalWrite(PWM_RAIL3_OUT_PIN, false);
    pinMode(PWM_RAIL4_OUT_PIN, OUTPUT);
    digitalWrite(PWM_RAIL4_OUT_PIN, false);

    pinMode(PWM_RAILEN_OUT_PIN, OUTPUT);
    digitalWrite(PWM_RAILEN_OUT_PIN, false);

    isrPWMdirectionNext = false;
    isrPWMdirectionNow = false;
    isrPWMvalueNext = 0;
    isrPWMvalueNow = 0;
    isrPWMplus = false;

    // Create semaphore to inform us when the timer has fired
    timerSemaphore = xSemaphoreCreateBinary();

    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).
    timer = timerBegin(0, 80, true);

    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer, &onTimer, true);

    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, 1000000, true);

    // Start an alarm
    timerAlarmEnable(timer);

}

void SetPWMCommand(bool direction, uint16_t pwmValue)
{
    portENTER_CRITICAL(&timerMux);
    isrPWMdirectionNext = direction;
    isrPWMvalueNext = pwmValue;
    portEXIT_CRITICAL(&timerMux);
}

void SetPWMCommand(uint16_t pwmValue)
{
    portENTER_CRITICAL(&timerMux);
    isrPWMvalueNext = pwmValue;
    portEXIT_CRITICAL(&timerMux);
}


void loopPWMCommander()
{

    // If Timer has fired
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE) {
        uint32_t isrCount = 0, isrTime = 0;
        int64_t  bootTime = 0;
        bool PWMdirection = false;
        uint16_t PWMvalue = 0;
        // Read the interrupt count and time
        portENTER_CRITICAL(&timerMux);
        isrCount = isrCounter;
        bootTime = isrBootTime;
        PWMdirection = isrPWMdirectionNow;
        PWMvalue = isrPWMvalueNow;
        portEXIT_CRITICAL(&timerMux);
        // Print it
        Serial.print("onTimer no. ");
        Serial.print(isrCount);
        Serial.print(", dir:");
        if (PWMdirection)
            Serial.print("Forward  ");
        else
            Serial.print("Backward ");
        Serial.print(", dutyCycle:");
        Serial.print((int32_t)PWMvalue);
        Serial.print(", boot time:");
        Serial.print((int32_t)bootTime);
        Serial.println(" us");
    }

}
