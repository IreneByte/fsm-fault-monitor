/*
  v3 - FSM-Based Monitoring System

  Features:
  - Push button state control
  - Input fault detection
  - LCD status display
  - RGB LED status indication
  - Active buzzer alarm during fault state
*/

#include <LiquidCrystal.h>
#include <DHT.h>
#include <NewPing.h>

#define DHTTYPE DHT22
#define MAX_DISTANCE 200

//* PIN CONFIGURATIONS
// LCD pin configuration
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

const int trigPin = A3;
const int echoPin = A2;
const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;
const int buzzerPin = 2;
const int buttonPin = A4;
const int faultPin = A5;
const int photoresPin = A0;
const int tempHumidPin = 4;

//* COMPONENT CONFIGURATIONS
// Ultrasonic pin configuration
NewPing sensor(trigPin, echoPin, MAX_DISTANCE);
long distance;
unsigned long objectDetectedTime = 0;

// Temperature and humidity sensor
DHT dht(tempHumidPin, DHTTYPE);
float temperature;

// Sensor thresholds
const int distanceThreshold = 10;
const int lightThreshold = 200;
const float tempThreshold = 30.0;

// System states
enum State {
  IDLE,
  RUNNING, 
  FAULT, // Buzzer on, light red
  RESET_REQUIRED // Problem fixed, waiting to restart
};

// Tracks current and previous system states
State currentState = RUNNING;
State lastState = RESET_REQUIRED;

// Fault statuses
const char* currentFault = "";
const char* lastFault = "INIT";

// Tracks previous button state for edge detection
int lastButtonReading = HIGH;

// Debounce time
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

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

  runSelfTest();
}

void loop() {
  handleButton();
  readSensors();
  testFault();
  updateLCD();
  updateOutputs();
}

void runSelfTest() {
  int lightLevel = analogRead(photoresPin);

  bool dhtFailed = isnan(dht.readTemperature());
  // SR04 not tested since ping_cm() returns 0 for both out of range and disconnected -> Indistinguishable without extra hardware
  bool ldrFailed = (lightLevel == 0 || lightLevel == 1023);

  testSensor("TESTING DHT...", dhtFailed);
  testSensor("TESTING LDR...", ldrFailed);
  
  lcd.clear();

  bool failFlag = dhtFailed || ldrFailed;
  currentState = (failFlag) ? IDLE : RUNNING;
}

void testSensor(const char* name, bool failed) {
  const int startDelay = 1500;
  
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(name);

  delay(startDelay);
  lcd.setCursor(0, 1);

  lcd.print((failed) ? "FAIL" : "OK");
  delay(startDelay);
}

void readSensors() {
  if (currentState == RUNNING) {
    // Ultrasonic
    distance = sensor.ping_cm();

    // Photoresistor
    int lightLevel = analogRead(photoresPin);

    // Temperature
    temperature = dht.readTemperature();
    if (isnan(temperature)) return;

    // Fault Detection
    // F01 - Object too close
    if (distance < distanceThreshold) {
      if (objectDetectedTime == 0) {
        objectDetectedTime = millis();
      }
      if (millis() - objectDetectedTime > 2000) {
        currentFault = "F01 OBJ JAM";
        currentState = FAULT;
      }
    } else {
      objectDetectedTime = 0;
    }

    // F02 - High temperature
    if (temperature > tempThreshold)
    {
      currentFault = "F02 HIGH TEMP";
      currentState = FAULT;
    } 

    // F03 - Blocked light sensor
    if (lightLevel < lightThreshold)
    {
      currentFault = "F03 LOW LIGHT";
      currentState = FAULT;
    }
  } else {
    objectDetectedTime = 0;
  }
}

// Handles button input and state transitions
void handleButton() {
  // Read current button state
  int currentButtonReading = digitalRead(buttonPin);
  static bool hasTriggered = false;

  // Reset debounce timer if reading changes
  if (currentButtonReading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = currentButtonReading;
  }

  // Only accept stable button presses
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Detects a new button press
    if (currentButtonReading == LOW && !hasTriggered) {
      // State transition logic
      switch (currentState) {
        case IDLE:
          currentState = RUNNING;
          break;

        case RUNNING:
          currentState = IDLE;
          break;

        case FAULT:
          currentState = RESET_REQUIRED;
          break;

        case RESET_REQUIRED:
          currentState = RUNNING;
          currentFault = "";
          lastFault = "";
          break;

        default:
          break;
      }
      hasTriggered = true;
    }
    if (currentButtonReading == HIGH) {
      hasTriggered = false;
    }
  }
}

// Button that creates test fault
void testFault() {
  // Fault can only trigger while system is running
  if (currentState == RUNNING && digitalRead(faultPin) == LOW) {
    currentFault = "F0 TEST FAULT";
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
    switch (currentState) {
      case IDLE:
        lcd.print("SYS: IDLE");
        printRunTime("IDLE");
        break;
      
      case RUNNING:
        lcd.print("SYS: RUNNING");
        printRunTime("RUNNING");
        break;

      case FAULT:
        lcd.print("SYS: FAULT!");
        printRunTime("FAULT");
        break;

      case RESET_REQUIRED:
        lcd.print("SYS: RESET");
        printRunTime("RESET REQUIRED");
        break;

      default:
        break;
    }
    // Save current state for future comparison
    lastState = currentState;
  }
  
  if (strcmp(currentFault, lastFault) != 0) {
    lcd.setCursor(0, 1);
    lcd.print(currentFault);

    lastFault = currentFault;
  }
}

void printRunTime(const char* state) {
  unsigned long runTime = millis();

  Serial.print("[");
  Serial.print(runTime);
  Serial.print("] STATE: ");
  Serial.println(state);

}

// Controls LED and buzzer outputs based on system state
void updateOutputs() {
  // Output control logic for each state
  switch (currentState) {
    case IDLE:
      // System inactive
      setColor(0, 0, 255); // Blue
      digitalWrite(buzzerPin, LOW);
      break;
    
    case RUNNING:
      // Normal operation
      setColor(0, 255, 0); // Green
      digitalWrite(buzzerPin, LOW);  
      break; 

    case FAULT:
      // Fault detected
      setColor(255, 0, 0); // Red
      digitalWrite(buzzerPin, HIGH);  
      break;

    case RESET_REQUIRED:
      // Waiting for manual reset
      setColor(255, 70, 0); // Orange
      digitalWrite(buzzerPin, LOW);
      break;
    
    default:
      break;
  }
}

// Controls RGB LED color using PWM
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}