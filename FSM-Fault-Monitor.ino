/*
  v2 - FSM-Based Monitoring System

  Features:
  - Push button state control
  - Input fault detection
  - LCD status display
  - RGB LED status indication
  - Active buzzer alarm during fault state
*/

#include <LiquidCrystal.h>
#include <DHT.h>
#include "SR04.h"

#define DHTTYPE DHT11

// LCD pin configuration
LiquidCrystal lcd(9, 10, A3, A2, A1, A0);

// Ultrasonic sensor pins
int trigPin = A5;
int echoPin = 11;

// Ultrasonic pin configuration
SR04 sensor = SR04(echoPin, trigPin);
long distance;
unsigned long objectDetectedTime = 0;

// Photoresistor data pin
int photoresPin = A4;

// Temperature and humidity sensor pin
int tempHumidPin = 4;
DHT dht(tempHumidPin, DHTTYPE);
float temperature;

// Sensor thresholds
int distanceThreshold = 10;
int lightThreshold = 200;
float tempThreshold = 30.0;

// Warning flags
bool warningTemp = false;
bool warningLight = false;

// Output pins
int redPin = 6;
int greenPin = 5;
int bluePin = 3;
int buzzerPin = 8;

// Input pins
int buttonPin = 2;
int faultPin = 7;

// System states
enum State {
  IDLE,
  RUNNING, 
  FAULT,
  RESET_REQUIRED
};

// Tracks current and previous system states
State currentState = IDLE;
State lastState = RESET_REQUIRED;

// Fault and warning statuses
const char* currentStatus = "ALL NORMAL";
const char* lastStatus = "";

// Tracks previous button state for edge detection
int lastButtonReading = HIGH;

// Debounce times
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  // Configure input pins using internal pull-up resistors
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(faultPin, INPUT_PULLUP);

  // Configure output pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Initialize LCD display
  lcd.begin(16, 2);

  dht.begin();
  Serial.begin(9600);
}

void loop() {
  handleButton();
  readSensors();
  checkFault();
  updateLCD();
  updateOutputs();
}

void readSensors() {
  if (currentState == RUNNING) {
    // Ultrasonic
    distance = sensor.Distance();

    // Photoresistor
    int lightLevel = analogRead(photoresPin);

    // Temperature
    temperature = dht.readTemperature();

    // Serial monitoring
    Serial.print("Temp: ");
    Serial.println(temperature);

    Serial.print("Distance: ");
    Serial.println(distance);

    // Fault Detection
    // F01 - Object too close
    if (distance < distanceThreshold) {
      if (objectDetectedTime == 0) {
        objectDetectedTime = millis();
      }
      if (millis() - objectDetectedTime > 2000) {
        currentState = FAULT;
      }
    } else {
      objectDetectedTime = 0;
    }

    // Warning Detection
    // W01 - High temperature
    if (temperature > tempThreshold)
    {
      warningTemp = true;
    } else {
      warningTemp = false;
    }

    // W02 - Blocked light sensor
    if (lightLevel < lightThreshold)
    {
      warningLight = true;
    } else {
      warningLight = false;
    }
  } else {
    warningTemp = false;
    warningLight = false;
    objectDetectedTime = 0;
  }
}

// Handles button input and state transitions
void handleButton() {
  // Read current button state
  int currentButtonReading = digitalRead(buttonPin);

  // Reset debounce timer if reading changes
  if (currentButtonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  // Only accept stable button presses
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Detects a new button press
    if (currentButtonReading == LOW && lastButtonReading == HIGH) {
      // State transition logic
      if (currentState == IDLE) {
        currentState = RUNNING;
      } 
      else if (currentState == RUNNING) {
        currentState = IDLE;
      }
      else if (currentState == FAULT) {
        currentState = RESET_REQUIRED;
      }
      else if (currentState == RESET_REQUIRED) {
        currentState = IDLE;
      }
    }
  }

  // Store button state for next loop iteration
  lastButtonReading = currentButtonReading;
}

// Checks for fault conditions during operation
void checkFault() {
  // Fault can only trigger while system is running
  if (currentState == RUNNING && digitalRead(faultPin) == LOW) {
    currentState = FAULT;
  }
}

// Updates the LCD display when the state changes
void updateLCD() {
  // Only update LCD when state changes to prevent flickering
  if (currentState != lastState) {
    lcd.clear();
    
    lcd.setCursor(0,0);

    // Display current system state on LCD
    if (currentState == IDLE)
    {
      lcd.print("SYS: IDLE");
    }
    else if (currentState == RUNNING)
    {
      lcd.print("SYS: RUNNING");
    }
    else if (currentState == FAULT)
    {
      lcd.print("SYS: FAULT!");
    }
    else if (currentState == RESET_REQUIRED)
    {
      lcd.print("SYS: RESET!");
    }

    // Save current state for future comparison
    lastState = currentState;
  }

  if (currentState == FAULT)
  {
    currentStatus = "F01 OBJECT JAM";
  }
  else if (warningTemp)
  {
    currentStatus = "W01 HIGH TEMP";
  }
  else if (warningLight)
  {
    currentStatus = "W02 LOW LIGHT";
  }
  else {
    currentStatus = "ALL NORMAL";
  }
  
  if (currentStatus != lastStatus) {
    lcd.setCursor(0,1);
    lcd.print(currentStatus);

    lastStatus = currentStatus;
  }
}

// Controls LED and buzzer outputs based on system state
void updateOutputs() {
  // Output control logic for each state
  if (currentState == IDLE) {
    // System inactive
    setColor(0, 0, 0); // Off
    digitalWrite(buzzerPin, LOW);
  } 
  else if (currentState == RUNNING) {
    // Normal operation
    if (warningTemp || warningLight) {
      setColor(255, 255, 0); // Yellow warning
    }
    else {
      setColor(0, 255, 0); // Green normal
    }
    digitalWrite(buzzerPin, LOW);    
  }
  else if (currentState == FAULT) {
    // Fault detected
    setColor(255, 0, 0); // Red
    digitalWrite(buzzerPin, HIGH);    
  }
  else if (currentState == RESET_REQUIRED) {
    // Waiting for manual reset
    setColor(255, 70, 0); // Orange
    digitalWrite(buzzerPin, LOW);
  }
}

// Controls RGB LED color using PWM
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}