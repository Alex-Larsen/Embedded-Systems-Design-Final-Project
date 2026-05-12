#include <Wire.h>
#include <LiquidCrystal.h>
#include <string.h>
#include <RTClib.h>
#include <INA226_WE.h>
#include "PinChangeInterrupt.h"

// Register Based Implementation -------------------------------------------------------------------

#define RDA 0x80
#define TBE 0x20  

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}

unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}

unsigned char U0getchar()
{
  return *myUDR0;
}

void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

void U0print(String data, bool newLine) {
  for (int i = 0; i < data.length(); i++) {
    char c = data.charAt(i);
    U0putchar(c);
  }

  if (newLine == true) {
    U0putchar('\n');
  }
}

void adc_init()  
{
  *my_ADCSRA |= 0b10000000;
  *my_ADCSRA &= 0b11010000;
  *my_ADCSRB &= 0b11110000;
  *my_ADMUX |= 0b01000000;
  *my_ADMUX &= 0b01000000;
}

unsigned int adc_read(unsigned char adc_channel_num) 
{
  if (adc_channel_num == 0) {
    *my_ADMUX &= 0b11100000;
    *my_ADCSRB &= 0b11110111;
    *my_ADCSRA |= 0b01000000;
    while((*my_ADCSRA & 0x40) != 0);
  }
  unsigned int value = (*my_ADC_DATA & 0x03FF);
  return value;
}

unsigned int digRead(unsigned char* pinRegister, const char pin) {
  return (*pinRegister & pin) ? 1 : 0;
}

void modePullup(unsigned char* pinRegister, unsigned char* pinPort, const char pin) {
  *pinRegister &= ~pin;
  *pinPort |= pin;
}

void modeInput(unsigned char* pinRegister, unsigned char* pinPort, const char pin) {
  *pinRegister &= ~pin;
  *pinPort &= !pin;
}

void modeOutput(unsigned char* pinRegister, unsigned char* pinPort, const char pin) {
  *pinRegister |= pin;
}

// Port Declaration -------------------------------------------------------------------------------

unsigned char* ddr_b = (unsigned char*) 0x24;
unsigned char* port_b = (unsigned char*) 0x25;
unsigned char* pin_b = (unsigned char*) 0x23;

unsigned char* ddr_h = (unsigned char*) 0x101;
unsigned char* port_h = (unsigned char*) 0x102;
unsigned char* pin_h = (unsigned char*) 0x100;

unsigned char* ddr_c = (unsigned char*) 0x27;
unsigned char* port_c = (unsigned char*) 0x28;
unsigned char* pin_c = (unsigned char*) 0x26;

unsigned char* ddr_e = (unsigned char*) 0x2D;
unsigned char* port_e = (unsigned char*) 0x2E;
unsigned char* pin_e = (unsigned char*) 0x2C;

unsigned char* ddr_k = (unsigned char*) 0x107;
unsigned char* port_k = (unsigned char*) 0x108;
unsigned char* pin_k = (unsigned char*) 0x106;

unsigned char* ddr_l = (unsigned char*) 0x10A;
unsigned char* port_l = (unsigned char*) 0x10B;
unsigned char* pin_l = (unsigned char*) 0x109;

unsigned char* ddr_d = (unsigned char*) 0x2A;
unsigned char* port_d = (unsigned char*) 0x2B;
unsigned char* pin_d = (unsigned char*) 0x29;

unsigned char* ddr_g = (unsigned char*) 0x33;
unsigned char* port_g = (unsigned char*) 0x34;
unsigned char* pin_g = (unsigned char*) 0x32;

unsigned char* ddr_f = (unsigned char*) 0x30;
unsigned char* port_f = (unsigned char*) 0x31;
unsigned char* pin_f = (unsigned char*) 0x2F;

// LCD Definitions ---------------------------------------------------------------------------------

const int RS = 52, EN = 53, D4 = 50, D5 = 51, D6 = 48, D7 = 49;
int lcdDelay = 0;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void lcdPrintUpper(String stringData) {
  int col = 0, row = 0;
  char data[stringData.length()];

  for (int counter = 0; counter < stringData.length(); counter++) {
    data[counter] = stringData.charAt(counter);
  }
  
  for (int counter = 0; counter < (sizeof(data) / sizeof(data[0])); counter++) {
    lcd.setCursor(col, row);
    lcd.write(data[counter]);
    col++;
  }

  col = 0;
}

void lcdPrintLower(String stringData) {
  int col = 0, row = 1;
  char data[stringData.length()];

  for (int counter = 0; counter < stringData.length(); counter++) {
    data[counter] = stringData.charAt(counter);
  }
  
  for (int counter = 0; counter < (sizeof(data) / sizeof(data[0])); counter++) {
    lcd.setCursor(col, row);
    lcd.write(data[counter]);
    col++;
  }

  col = 0;
}

// RTC Defintions ----------------------------------------------------------------------------------

RTC_DS1307 rtcModule;

void rtcPrint() {
  DateTime currentTime = rtcModule.now();

  char buffer[20];
  sprintf(buffer, "%02d:%02d:%02d", currentTime.hour(), currentTime.minute(), currentTime.second());
  String clockData = "Clock: " + String(buffer);

  U0print(clockData, false);
}

// INA Defintions

#define I2C_ADDRESS1 0x40

INA226_WE ina226a(I2C_ADDRESS1);

unsigned long lastSensorUpdate = -60000; 
const unsigned long sensorInterval = 60000;

void sensorPrint(float ecSign1, float ecSign2, float ecSign3, float ecSign4) {
  float amperage = ina226a.getCurrent_mA();
  char sign1 = ecSign1 >= 0 ? '+' : '-'; 
  char sign2 = ecSign2 >= 0 ? '+' : '-';
  char sign3 = ecSign3 >= 0 ? '+' : '-'; 
  char sign4 = ecSign4 >= 0 ? '+' : '-';
  lcdPrintUpper("mA:" + String(amperage) + " EC:" + sign1 + sign2 + sign3 + sign4);
}

// PWM Defintions
  // Index & Middle
#define PWM6 4
#define PWM8 5
  // Ring & Pinky
#define PWM3 3
#define PWM4 2

int pwmValue = 30;

void setPWM(int pwm, char pin1, char pin2) {
  analogWrite(pin1, pwm);
  analogWrite(pin2, pwm);
} 

// DIR Definitions

#define DIR6 32
#define DIR8 35

#define DIR3 36
#define DIR4 37

const char dir6 = (0x01 << 5); // PC5
const char dir8 = (0x01 << 2); // PC2
const char dir3 = (0x01 << 1); // PC1
const char dir4 = (0x01 << 0); // PC0

void actuate(char direction) {
  if (direction == 'E' || direction == 'e') {
    *port_c ^= dir6;
  }
  else if (direction == 'R' || direction == 'r') {
    *port_c ^= dir8;
  }
  else if (direction == 'T') {
    *port_c ^= dir3;
  }
  else if (direction == 'Y') {
    *port_c ^= dir4;
  }
}

// Encoder Definitions

  // Index Encoders
#define ENCA3 A8 // Extension Motor
#define ENCB3 A9

#define ENCA4 A10 // Flexion Motor
#define ENCB4 A11

//   // Ring Encoders
#define ENCA5 A12 // Flexion Motor
#define ENCB5 A13

#define ENCA6 A14 // Extension Motor
#define ENCB6 A15

const char enca3 = (0x01 << 0); // PK0
const char encb3 = (0x01 << 1); // PK1
const char enca4 = (0x01 << 2); // PK2
const char encb4 = (0x01 << 3); // PK3
const char enca5 = (0x01 << 4); // PK4
const char encb5 = (0x01 << 5); // PK5
const char enca6 = (0x01 << 6); // PK6
const char encb6 = (0x01 << 7); // PK7

class encoderISR {
  private:
    int A, B;
    int lastA = 0;
    int lastB = 0;
    volatile long &count;
    const char encA, encB;
  public:
    encoderISR(const char encPin1, const char encPin2, volatile long &encoderCount) : count(encoderCount), encA(encPin1), encB(encPin2) {}

    void initiate() {
      modePullup(ddr_k, port_k, encA);
      modePullup(ddr_k, port_k, encB);
      lastA = digRead(pin_k, encA);
      lastB = digRead(pin_k, encB);
    }

    void update() {
      A = digRead(pin_k, encA);
      B = digRead(pin_k, encB);

      if (A != lastA || B != lastB) {
        if ((lastA == 0 && A == 1 && B == 0) || 
            (lastA == 1 && A == 0 && B == 1) ||
            (lastB == 0 && B == 1 && A == 1) || 
            (lastB == 1 && B == 0 && A == 0)) {
          count += 1;
        } else {
          count -= 1;
        }
      }
      
      lastA = A;
      lastB = B;
    }

};

// Encoder Related Variables

volatile long encoderCount3 = 0;
volatile long encoderCount4 = 0;
volatile long encoderCount5 = 0;
volatile long encoderCount6 = 0;

volatile long prevEC3 = 0;
volatile long prevEC4 = 0;
volatile long prevEC5 = 0;
volatile long prevEC6 = 0;

encoderISR ISR3(enca3, encb3, encoderCount3);
encoderISR ISR4(enca4, encb4, encoderCount4);
encoderISR ISR5(enca5, encb5, encoderCount5);
encoderISR ISR6(enca6, encb6, encoderCount6);

// LED Potentiometer Definitions -------------------------------------------------------------------

#define brightnessIn A0

const int offLED = 13, onLED = 12, errorLED = 11, idleLED = 10, 
  actuateLED = 9, autoLED = 8, manualLED = 7, resetLED = 6;

const char brightnessControl = (0x01 << 0); // PF0
const char OFF_LED = (0x01 << 7); // PB7
const char ON_LED = (0x01 << 6); // PB6
const char ERR_LED = (0x01 << 5); // PB5
const char IDLE_LED = (0x01 << 4); // PB4
const char ACT_LED = (0x01 << 6); // PH6
const char AUTO_LED = (0x01 << 5); // PH5
const char MNL_LED = (0x01 << 4); // PH4
const char RES_LED = (0x01 << 3); // PH3

const int ledList[] = {
  offLED,
  onLED,
  errorLED,
  idleLED,
  actuateLED,
  autoLED,
  manualLED,
  resetLED
};

float brightnessPercentage = 0;

void ledOFF() {
  for (int counter = 0; counter < (sizeof(ledList) / sizeof(ledList[0])); counter++) {
    analogWrite(ledList[counter], 0);
  }
}

// Button Definitions ------------------------------------------------------------------------------

#define powerButton 19
#define modeButton 41
#define resetButton 40

const char powerSwitch = (0x01 << 2); // PD2
const char modeSwitch = (0x01 << 0); // PG0
const char resetSwitch = (0x01 << 1); // PG1

int buttonDelay = 0;
unsigned long prevStartInterrupt = 0;
volatile bool startButton = false;

void startISR() {
  unsigned long currStartInterrupt = millis();

  if (currStartInterrupt - prevStartInterrupt > buttonDelay) {
    startButton = true;
  }

  prevStartInterrupt = currStartInterrupt;
}

// State Declaration -------------------------------------------------------------------------------

String deviceStates[] {
  "OFF",
  "ON",
  "ERR",
  "IDLE",
  "ACT",
  "AUTO",
  "MNL",
  "RES"
};

String systemState = deviceStates[0];
String actuationState = deviceStates[3];
String deviceMode = deviceStates[5];

// Delay Functions ---------------------------------------------------------------------------------

void delayDecrement(int &delay) {
  delay == 0 ? delay : delay -= 1;
}

// State Functions ---------------------------------------------------------------------------------

void displayCurrentState() {
  lcdPrintLower("                ");
  ledOFF();
  String displayState = "St: ";

  for (int counter = 0; counter < sizeof(deviceStates) / sizeof(deviceStates[0]); counter++) {
    if (deviceMode == "RES") {
      displayState = "St: " + deviceMode;
      analogWrite(ledList[7], brightnessPercentage * 255);
      break;
    }

    if (systemState == deviceStates[counter]) {
      if (systemState == deviceStates[2] || systemState == deviceStates[0]) {
        displayState = "St: " + systemState;
        analogWrite(ledList[counter], brightnessPercentage * 255);
        break;
      }
      displayState += systemState + "/";
      analogWrite(ledList[counter], brightnessPercentage * 255);
    } 

    if (actuationState == deviceStates[counter]) {
      displayState += actuationState + "/";
      analogWrite(ledList[counter], brightnessPercentage * 255);
    }

    if (deviceMode == deviceStates[counter]) {
      displayState += deviceMode;
      analogWrite(ledList[counter], brightnessPercentage * 255);
    }

  }
  lcdPrintLower(displayState);
}

// Manual Variables --------------------------------------------------------------------------------

int manualState = 0;
int pwmDelay = 0;
int clockDelay = 0;

void manualActuate(String mode, int &state, int &clkDelay, int &pwm, char pin1, char pin2, int &pwmDelay, long ec1, long ec2, char cmd1, char cmd2) {
  if (mode == "MNL") {
    if (state == 0 && clkDelay == 0) {
      if (ec1 != 0 && ec2 == 0) {
        pwm = 80;
        setPWM(pwm, pin1, pin2);
        actuate(cmd1);
        actuate(cmd2);
        actuate(cmd2);
        pwm = 30;
        setPWM(pwm, pin1, pin2);
        
        clkDelay = 50;
        state = 1;
      } else if (ec1 == 0 && ec2 != 0) {
        pwm = 80;
        setPWM(pwm, pin1, pin2);
        actuate(cmd2);
        actuate(cmd1);
        actuate(cmd1);
        pwm = 30;
        setPWM(pwm, pin1, pin2);
        clkDelay = 50;
        state = 2;
      } 
    } else if (state == 1 && ec1 == 0 && clkDelay == 0) {
      actuate(cmd1);
      clkDelay = 50;
      pwmDelay = 30;

      state = 3;
    } else if (state == 2 && ec2 == 0 && clkDelay == 0) {
      actuate(cmd2);
      clkDelay = 50;
      pwmDelay = 30;

      state = 3;
    } else if (state == 3 && ec1 == 0 && ec2 == 0 && pwmDelay == 0) {
      pwm = 30;
      setPWM(pwm, pin1, pin2);
      state = 0;
    }
  }
}

// Reset Function ----------------------------------------------------------------------------------

struct Exo {
  long flexPosition;
  long extPosition;
  long flexError;
  long extError;
  char flexDir;
  char extDir;
  char flexPWM;
  char extPWM;
};

Exo middleFinger, ringFinger;

const char motorDirections[] = 
{
  middleFinger.flexDir = dir8, middleFinger.extDir = dir6,
  ringFinger.flexDir = dir4, ringFinger.extDir  = dir3
};

char motorPWMs[] = 
{
  middleFinger.flexPWM = PWM8, middleFinger.extPWM = PWM6,
  ringFinger.flexPWM = PWM4, ringFinger.extPWM = PWM3
};

void resetExo(String &mode) {
  long targetPos = 0;
  bool allZero = true;

  noInterrupts();
  long currentPositions[] = 
  {
    middleFinger.flexPosition = encoderCount3, middleFinger.extPosition = encoderCount4,
    ringFinger.flexPosition = encoderCount5, ringFinger.extPosition = encoderCount6,
  };
  interrupts();

  for (int counter = 0; counter < (sizeof(currentPositions) / sizeof(currentPositions[0])); counter++){
    long error = currentPositions[counter] - targetPos;
    if (abs(error) > 10) {
      analogWrite(motorPWMs[counter], pwmValue);

      if (error > 0) {
        *port_c |= motorDirections[counter];
      } else {
        *port_c &= !(motorDirections[counter]);
      }
      
      allZero = false;
    } else {
      analogWrite(motorPWMs[counter], 0);
    }
  }
  
  if (allZero == true) {
    mode = "AUTO";
  }

}

// Setup Function ----------------------------------------------------------------------------------

void setup() {
  // UART Implementation

  U0init(9600);

  // ADC Implementation

  adc_init();

  // I2C Implementation

  Wire.begin();
  Wire.setClock(400000);

  bool ina1_ok = ina226a.init();

  if (!ina1_ok) {
    U0print("INA226 #1 Initialization: FAIL", true);
    systemState = "ERR";
  }

  if (ina1_ok) U0print("INA226 #1 Initialization: PASS", true);

  ina226a.setAverage(INA226_AVERAGE_1);
  ina226a.setConversionTime(INA226_CONV_TIME_1100);
  ina226a.setResistorRange(0.00635, 10.0); 
  ina226a.setCorrectionFactor(0.93);

  U0print("INA226 Current Sensor Example Sketch - Continuous", true);

  ina226a.waitUntilConversionCompleted(); 
  
  // LCD Initialization

  lcd.begin(16, 2); // set up number of columns and rows

  // RTC Checks

  if (!rtcModule.begin()) {
    U0print("RTC: Failed Initialization", true);
    systemState = "ERR";
  }

  if (!rtcModule.isrunning()) {
    U0print("RTC: NOT Synchronized", true);
    rtcModule.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  }

  // Start Button Interrupt

  attachInterrupt(digitalPinToInterrupt(powerButton), startISR, FALLING);
  
  // Encoder Interrupts

  ISR3.initiate();
  ISR4.initiate();
  ISR5.initiate();
  ISR6.initiate();

  attachPCINT(digitalPinToPCINT(ENCA3), []{ISR3.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCB3), []{ISR3.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCA4), []{ISR4.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCB4), []{ISR4.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCA5), []{ISR5.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCB5), []{ISR5.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCA6), []{ISR6.update();}, CHANGE);
  attachPCINT(digitalPinToPCINT(ENCB6), []{ISR6.update();}, CHANGE);

  // Pin Setups
  modeInput(ddr_f, port_f, brightnessControl);
  modeOutput(ddr_b, port_b, OFF_LED);
  modeOutput(ddr_b, port_b, ON_LED);
  modeOutput(ddr_b, port_b, ERR_LED);
  modeOutput(ddr_b, port_b, IDLE_LED);

  modeOutput(ddr_h, port_h, ACT_LED);
  modeOutput(ddr_h, port_h, AUTO_LED);
  modeOutput(ddr_h, port_h, MNL_LED);
  modeOutput(ddr_h, port_h, RES_LED);

  modeInput(ddr_d, port_d, powerSwitch);
  modeInput(ddr_g, port_g, modeSwitch);
  modeInput(ddr_g, port_g, resetSwitch);

  modeOutput(ddr_c, port_c, dir6);
  modeOutput(ddr_c, port_c, dir8);
  modeOutput(ddr_c, port_c, dir3);
  modeOutput(ddr_c, port_c, dir4);

  *port_c &= !dir3;
  *port_c &= !dir4;
  *port_c &= !dir6;
  *port_c &= !dir8;
}

// Loop Function ------------------------------------------------------------------------------------

void loop() {
  noInterrupts();
  float currentEC3 = encoderCount3;
  float currentEC4 = encoderCount4;
  float currentEC5 = encoderCount5;
  float currentEC6 = encoderCount6;
  interrupts();

  long deltaEC3 = abs(currentEC3) - abs(prevEC3);
  long deltaEC4 = abs(currentEC4) - abs(prevEC4);
  long deltaEC5 = abs(currentEC5) - abs(prevEC5);
  long deltaEC6 = abs(currentEC6) - abs(prevEC6);
  rtcPrint();

  if (millis() - lastSensorUpdate >= sensorInterval) {
    sensorPrint(currentEC3, currentEC4, currentEC5, currentEC6);
    lastSensorUpdate = millis(); 
  }

  if (lcdDelay == 0) {
    displayCurrentState();
    lcdDelay = 5;
  }
  
  bool modeButtonPressed = digRead(pin_g, modeSwitch), resetButtonPressed = digRead(pin_g, resetSwitch); 

  brightnessPercentage = (float)adc_read(0) / 1023.00;

  if (buttonDelay == 0) {

    if (startButton == true && systemState == "OFF" ) {
      systemState = "ON";
      buttonDelay = 20;
    } else if (startButton == true && systemState == "ON") {
      systemState = "OFF";
      buttonDelay = 20;
    }
    startButton = false;

    if (modeButtonPressed == true && deviceMode == "AUTO") {
      deviceMode = "MNL";
      buttonDelay = 20;
    } else if (modeButtonPressed == true && deviceMode == "MNL") {
      deviceMode = "AUTO";
      buttonDelay = 20;
    }

    if (resetButtonPressed == true && deviceMode != "RES") {
      deviceMode = "RES";
      buttonDelay = 20;
    } else if (resetButtonPressed && deviceMode == "RES") {
      deviceMode = "AUTO";
      buttonDelay = 20;
    }

  }
  
  delayDecrement(buttonDelay);
  delayDecrement(lcdDelay);
  delayDecrement(clockDelay);
  delayDecrement(pwmDelay);

  if (systemState != "OFF") {
    if (systemState != "ERR") {
      if (deviceMode == "RES") {
        resetExo(deviceMode);
      }
  
      if (deviceMode == "AUTO") {
        manualState = 0;
        char cmd;

        if (U0kbhit()) {
          cmd = U0getchar();
        }

        
        if (cmd == 'L') {
          pwmValue = 30;
        } else if (cmd == 'H') {
          pwmValue = 80;
        } else if (cmd == '2') {
          analogWrite(PWM6, 0);
          analogWrite(PWM8, 0);
        } else if (cmd == '3') {
          analogWrite(PWM3, 0);
          analogWrite(PWM4, 0);
        } else if (cmd == '6') {
          //Middle PWMs
          analogWrite(PWM6, pwmValue);
          analogWrite(PWM8, pwmValue);
        } else if (cmd == '7') {
          //Ring PWMs
          analogWrite(PWM3, pwmValue);
          analogWrite(PWM4, pwmValue);
        }

        actuate(cmd);
      } else if (deviceMode == "MNL") {
        manualActuate(deviceMode, manualState, clockDelay, pwmValue, PWM6, PWM8, pwmDelay, deltaEC3, deltaEC4, 'E', 'R');
        manualActuate(deviceMode, manualState, clockDelay, pwmValue, PWM3, PWM4, pwmDelay, deltaEC5, deltaEC6, 'T', 'Y');
      }

      if (deltaEC3 == 0 && deltaEC4 == 0 && deltaEC5 == 0 && deltaEC6 == 0) {
        actuationState = "IDLE";
      } else {
        actuationState = "ACT";
      }
    } else {
      setPWM(0, PWM6, PWM8);
      setPWM(0, PWM3, PWM4);
    }
  } else {
    setPWM(0, PWM6, PWM8);
    setPWM(0, PWM3, PWM4);
  }

  U0print(" | ", false);
  U0print(systemState, false);
  U0print(" | ", false);
  U0print(String(ina226a.getCurrent_mA()), false);
  U0print(" | ", false);
  U0print(String(currentEC3), false);
  U0print(" | ", false);
  U0print(String(currentEC4), false);
  U0print(" | ", false);
  U0print(String(currentEC5), false);
  U0print(" | ", false);
  U0print(String(currentEC6), false);
  U0print(" | ", false);
  U0print(String(brightnessPercentage * 100), true);

  prevEC3 = currentEC3;
  prevEC4 = currentEC4;
  prevEC5 = currentEC5;
  prevEC6 = currentEC6;
}


