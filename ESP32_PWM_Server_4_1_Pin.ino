// ESP32_PWM_Server_4_1_Pin.ino

#include "PWMCommander.h"
#include "PWMWebServer.h"

void setup() {
	Serial.begin(115200);
	SetupPWMCommander();
	SetupPWMWebServer();
}

void loop() {
	LoopPWMCommander();
	LoopPWMWebServer();
}
