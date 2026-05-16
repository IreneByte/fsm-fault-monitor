/*
  v1 - FSM-Based Monitoring System

  Features:
  - Push button state control
  - Input fault detection
  - LCD status display
  - RGB LED status indication
  - Active buzzer alarm during fault state
*/

#include <LiquidCrystal.h>

// LCD pin configuration
LiquidCrystal lcd(9, 10, A3, A2, A1, A0);

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
}

void loop() {
  handleButton();
  checkFault();
  updateLCD();
  updateOutputs();
}

// Handles button input and state transitions
void handleButton() {
  // Read current button state
  int currentbuttonReading = digitalRead(buttonPin);

  // Reset debounce timer if reading changes
  if (currentbuttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  // Only accept stable button presses
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Detects a new button press
    if (currentbuttonReading == LOW && lastButtonReading == HIGH) {
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
  lastButtonReading = currentbuttonReading;
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
      lcd.print("IDLE");
    }
    else if (currentState == RUNNING)
    {
      lcd.print("RUNNING...");
    }
    else if (currentState == FAULT)
    {
      lcd.print("FAULT!");
    }
    else if (currentState == RESET_REQUIRED)
    {
      lcd.print("RESET REQUIRED!");
    }

    // Save current state for future comparison
    lastState = currentState;
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
    setColor(0, 255, 0); // Green
    digitalWrite(buzzerPin, LOW);    
  }
  else if (currentState == FAULT) {
    // Fault detected
    setColor(255, 0, 0); // Red
    digitalWrite(buzzerPin, HIGH);    
  }
  else if (currentState == RESET_REQUIRED) {
    // Waiting for manual reset
    setColor(255, 165, 0); // Yellow-Orange
    digitalWrite(buzzerPin, LOW);
  }
}

// Controls RGB LED color using PWM
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}