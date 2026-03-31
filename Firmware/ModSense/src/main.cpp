#include <Arduino.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>

Adafruit_MCP4728 mcp;

// Functions
int getDistance();
int getIR();
void sendToDAC(int value);

// Constants
const int irPin = 26;
const int trigPin = 27; 
const int echoPin = 28;
const int dacSDAPin = 4;
const int dacSLCPin = 5;
const int cvA = MCP4728_CHANNEL_A;
const int cvB = MCP4728_CHANNEL_B;
const int cvC = MCP4728_CHANNEL_C;
const int cvD = MCP4728_CHANNEL_D;

// Variables
float duration, distance;  
bool isObjectDetected = false;

void sendToDAC(int channel, int value) {
  Serial.println("DAC OUT: CH " + String(channel) + " Value " + String(value));
  mcp.setChannelValue(channel, value);
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  return distance;
}

bool getIR() {
  isObjectDetected = digitalRead(irPin);
  return isObjectDetected;
}

void setup() {

  // Setup Pins + Serial
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  
  pinMode(irPin, INPUT);
	Serial.begin(9600);  
  Serial.println("Synthia ModSense Test!");

  // Try to initialize!
  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }

  sendToDAC(cvA, 4095);
  sendToDAC(cvB, 2048);
  sendToDAC(cvC, 1024);
  sendToDAC(cvD, 0);

}

void loop() {

  getDistance();
  getIR();
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("IR: ");
  Serial.println(isObjectDetected);
  delay(200);

}
