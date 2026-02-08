/*
 * Project: Ved-OS M4 Core (Sentinel)
 * Role: Physical Security Indicator (RGB LED)
 * Status: FIXED (Pin Definitions)
 */

#include <RPC.h>

// Onboard RGB LED Pins (Corrected for Giga M4)
#define LED_R LED_RED
#define LED_G LED_GREEN
#define LED_B LED_BLUE

int system_status = 0; // 0=Idle, 1=Connecting, 2=Secure, 3=Alert

void set_status(int status) {
  system_status = status;
}

void setup() {
  RPC.begin(); 
  
  // Bind function for M7 to call
  RPC.bind("set_status", set_status);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  // Initial State: Off (High is OFF on Giga LEDs)
  digitalWrite(LED_R, HIGH); 
  digitalWrite(LED_G, HIGH); 
  digitalWrite(LED_B, HIGH);
}

void loop() {
  if (system_status == 0) {
    // Idle: Slow White Breath
    digitalWrite(LED_R, LOW); digitalWrite(LED_G, LOW); digitalWrite(LED_B, LOW);
    delay(100);
    digitalWrite(LED_R, HIGH); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, HIGH);
    delay(2000);
  }
  else if (system_status == 1) {
    // Connecting: Blinking Blue
    digitalWrite(LED_R, HIGH); digitalWrite(LED_G, HIGH); 
    digitalWrite(LED_B, LOW); delay(200);
    digitalWrite(LED_B, HIGH); delay(200);
  }
  else if (system_status == 2) {
    // Secure: Solid Green
    digitalWrite(LED_R, HIGH); 
    digitalWrite(LED_G, LOW); 
    digitalWrite(LED_B, HIGH);
  }
  else if (system_status == 3) {
    // Alert: Fast Red Strobe
    digitalWrite(LED_R, LOW); digitalWrite(LED_G, HIGH); digitalWrite(LED_B, HIGH);
    delay(50);
    digitalWrite(LED_R, HIGH);
    delay(50);
  }
}