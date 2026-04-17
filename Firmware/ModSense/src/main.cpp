#include <Arduino.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 25
#endif

Adafruit_MCP4728 mcp;

const int irPin = 26;
const int trigPin = 27;
const int echoPin = 28;

const uint8_t segPins[] = {17,16,14,13,12,18,19}; // a b c d e f g dp
const bool COMMON_ANODE = true;
const uint8_t digitMap[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

void setSegment(uint8_t pin, bool on) {
  if (COMMON_ANODE) digitalWrite(pin, on ? LOW : HIGH);
  else digitalWrite(pin, on ? HIGH : LOW);
}

void showDigit(uint8_t d) {
  uint8_t mask = digitMap[d % 10];
  for (int i = 0; i < 8; i++) {
    setSegment(segPins[i], mask & (1 << i));
  }
}

//Encoder Setup
const int encA = 1;
const int encB = 0;
const int encSW = 2;


int encoderValue = 0;
int displayValue = 0;
int lastEncA = HIGH;


int readEncoderStep() {
  static int lastA = HIGH;
  static unsigned long lastStepTime = 0;
  const unsigned long debounceMs = 3;

  int a = digitalRead(encA);
  if (a == lastA) return 0;

  if (millis() - lastStepTime < debounceMs) {
    lastA = a;
    return 0;
  }

  lastStepTime = millis();
  lastA = a;

  if (digitalRead(encB) != a) return +1;
  return -1;
}

void updateEncoder() {
  static bool lastButton = HIGH;
  int step = readEncoderStep();
  if (step != 0) {
    encoderValue += step;

    if (encoderValue > 9) encoderValue = 9;
    if (encoderValue < 0) encoderValue = 0;

    displayValue = encoderValue;
  }
  bool button = digitalRead(encSW);
  if (lastButton == HIGH && button == LOW) {
    encoderValue = 0;
    displayValue = 0;
  }
  lastButton = button;
}

float distanceCm = 0;
int irValue = 0;

uint16_t clamp12(int v) {
  if (v < 0) return 0;
  if (v > 4095) return 4095;
  return (uint16_t)v;
}

void sendToDAC(MCP4728_channel_t channel, uint16_t value) {
  mcp.setChannelValue(channel, value);
}

float getDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1.0f;

  return (duration * 0.0343f) / 2.0f;
}

int getIR() {
  return analogRead(irPin);
}
const float distanceMin = 4.0f;
const float distanceMax = 70.0f;   // change this to extend range

uint16_t cvA(float d) {
  if (d < 0) return 0;
  d = constrain(d, distanceMin, distanceMax);
  float norm = (d - distanceMin) / (distanceMax - distanceMin); // 0..1
  float bipolar = (norm * 2.0f) - 1.0f;                           // -1..+1
  int dac = (int)(2048.0f + bipolar * 2047.0f);                  // centered
  //Serial.println("DAC: " + String(dac));
  return clamp12(dac);
}

uint16_t cvB(int ir) {
  return clamp12(ir);
}

uint16_t cvC(float d, int ir) {
  float a = (d < 0) ? 0.0f : constrain((d - 5.0f) / 95.0f, 0.0f, 1.0f);
  float b = constrain(ir / 4095.0f, 0.0f, 1.0f);
  return clamp12((int)((a * 0.7f + b * 0.3f) * 4095.0f));
}

uint16_t cvD() {
  return clamp12((millis() / 10) % 4096);
}

float cvAFiltered = 0;
float cvBFiltered = 0;
float cvCFiltered = 0;
float cvDFiltered = 0;



void statusOK() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void statusError() {
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

uint16_t sine(uint16_t input) {
  if (input < 20){
    input = 4095;
  }
  return (input-10);

}

float distanceFiltered = -1.0f;

float smoothValue(float current, float target, float alpha) {
  return current + alpha * (target - current);
}



void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  //Seven Segment Display
  for (auto pin : segPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  //Encoder Setup
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  pinMode(encSW, INPUT_PULLUP);
  lastEncA = digitalRead(encA);

  Serial.begin(115200);
  delay(4500);
  Serial.println("booting");

  if (!mcp.begin()) {
    statusError();
  Serial.println("MCP not found");
  }
  sendToDAC(MCP4728_CHANNEL_A, clamp12(0));
  showDigit(0);
  Serial.println("0");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_A, clamp12(1024));
  showDigit(1);
  Serial.println("1");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_A, 2048);
  showDigit(2);
  Serial.println("2");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_A, 4095);
  showDigit(3);
  Serial.println("3");
  delay(500);

  Serial.println("Testing Channel C");
  sendToDAC(MCP4728_CHANNEL_C, clamp12(0));
  showDigit(0);
  Serial.println("0");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_C, clamp12(1024));
  showDigit(1);
  Serial.println("1");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_C, 2048);
  showDigit(2);
  Serial.println("2");
  delay(1000);
  sendToDAC(MCP4728_CHANNEL_C, 4095);
  showDigit(3);
  Serial.println("3");
  delay(500);

  statusOK();
}

uint16_t output = 1024;

void loop() {
  float d = getDistanceCm();
  if (d >= 0) {
    if (distanceFiltered < 0) distanceFiltered = d;
    else distanceFiltered = smoothValue(distanceFiltered, d, 0.1f);
  }

  uint16_t aOut = cvA(distanceFiltered);

  output = sine(output);
  //Serial.println("a: " + String(aOut));
  sendToDAC(MCP4728_CHANNEL_A, aOut);
  sendToDAC(MCP4728_CHANNEL_B, aOut);
  sendToDAC(MCP4728_CHANNEL_C, aOut);

  //Serial.println("Setting DAC A+B to minimum value");
  //sendToDAC(MCP4728_CHANNEL_B, 0);
  //sendToDAC(MCP4728_CHANNEL_A, 0);
  //delay(4000);
  //Serial.println("Setting DAC A+B to maximum value");
  //sendToDAC(MCP4728_CHANNEL_B, 4095);
  //sendToDAC(MCP4728_CHANNEL_A, 4095);
  //delay(4000);
  
  //sendToDAC(MCP4728_CHANNEL_B, cvB(irValue));
  //sendToDAC(MCP4728_CHANNEL_C, cvC(distanceCm, irValue));
  //sendToDAC(MCP4728_CHANNEL_D, cvD());

  //TESTING encoder
  updateEncoder();
  showDigit(displayValue);
  //Serial.println(displayValue);

  statusOK();

  //delay(1);
}
