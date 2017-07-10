#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#define ONE_WIRE_BUS 13
#define DEBUG true
#define updateInterval 5000 //update the status every 5seconds
unsigned long lastUpdateTime = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/* Adress of 1-wire sensors can be found by following the tutorial at 
 *   http://henrysch.capnfatz.com/henrys-bench/arduino-temperature-measurements/ds18b20-arduino-user-manual-introduction-and-contents/ds18b20-user-manual-part-2-getting-the-device-address/
 */
DeviceAddress airTempSensor = {0x28, 0xFF, 0x7A, 0xF6, 0x82, 0x16, 0x03, 0x69};
DeviceAddress vatTempSensor = {0x28, 0xFF, 0xE3, 0x9C, 0x82, 0x16, 0x04, 0x25};
float airTemp;
float vatTemp;
unsigned long lastTempMeasurement = 0;
unsigned long error_interval = 300000; //5min in ms
const float THRESH = 0.5;

//short cycle timer
unsigned long RUN_THRESH = 120000; //2min in milliseconds, minimum time for heating/cooling elements to run

//bool for short cycle
bool can_turn_off = true;

float setTemp = 20; //desired temperature, default is 20deg
unsigned long start_time = 0; //variable for timing heating/cooling duration

//states to pass to action funtion declared here
const int STATE_IDLE = 0;
const int STATE_COOL = 1;
const int STATE_HEAT = 2;
int state = STATE_IDLE; //initialise by in idle mode

//Buttons for adjusting set temperature
const int incButtonPin = 8;
const int decButtonPin = 7;
unsigned long debounce_start = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;  // the debounce time; increase if the output flickers
const float TEMP_INCREMENT = 0.1;

//LCD display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  #if (DEBUG)
    Serial.begin(9600);
  #endif
  //set up buttons
  pinMode(incButtonPin, INPUT_PULLUP);
  digitalWrite(incButtonPin, HIGH);
  pinMode(decButtonPin, INPUT_PULLUP);
  digitalWrite(decButtonPin, HIGH);
  // Set up temperature probes
  sensors.setResolution(airTempSensor, 11); //resolution of 0.125deg cels, 
  sensors.setResolution(vatTempSensor, 11); //takes approx 375ms
  #if (DEBUG) 
    Serial.print("Vat sensor resolution: ");
    Serial.println(sensors.getResolution(vatTempSensor), DEC);
    Serial.print("Air sensor resolution: ");
    Serial.println(sensors.getResolution(airTempSensor), DEC);
  #endif
  lcd.begin(16,2);
  
}

void adjust_setTemp(){
  if(digitalRead(incButtonPin) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > debounceDelay) {
      setTemp += TEMP_INCREMENT;
      debounce_start = 0; //reset debounce timer
    }
  }
  if(digitalRead(decButtonPin) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > debounceDelay) {
      setTemp -= TEMP_INCREMENT;
      debounce_start = 0; //reset debounce timer
    }
  }
}
    
void activate_relay(String action) {
  //start timer so don't short cycle
  start_time = millis();
  #if (DEBUG) 
    Serial.println("Updated start_time");
    Serial.print("New start_time is: ");
    Serial.println(start_time);
  #endif
  can_turn_off = false;
  if(action == "cool") {
    #if (DEBUG)
      Serial.println("...Turning fridge on");
    #endif
    //write fridge relay pin high here (if active high)
  }
  if(action == "heat") {
    #if (DEBUG)
      Serial.println("...Turning heating on");
    #endif
    //write fridge relay pin high here (if active high)
  }
}

void proc_idle() {
  if (vatTemp - setTemp > THRESH) {
    state = STATE_COOL; //shift to checking if cooling can be switched off
    activate_relay("cool"); //activate cooling
  } else if (setTemp - vatTemp > THRESH) {
    state = STATE_HEAT;
    activate_relay("heat");
  } else {
  //write digital pins low here for relays

  }
}

void proc_heat() {
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  if ((vatTemp > setTemp) && (can_turn_off)) { 
    //if the temperature is within desired range
    //and minimum threshold has been met
    #if (DEBUG)
      Serial.println("...Switching heat off");
    #endif
    //start_time = 0;
    state = STATE_IDLE;
  }
}

void proc_cool() {
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  if ((vatTemp < setTemp) && (can_turn_off)) {
    #if (DEBUG)
      Serial.println("...Switching fridge off");
    #endif
    //start_time = 0;
    state = STATE_IDLE;
  }
}

void status_update() {
    if (state == STATE_HEAT) {
      Serial.print("Heater has been on for ");
      Serial.print((millis()-start_time)/1000);
      Serial.println(" seconds");
      if (can_turn_off) {
        Serial.println("Minimum time elapsed, heating can be turned off");
      }
    }else if (state == STATE_COOL) {
      Serial.print("Fridge has been on for ");
      Serial.print((millis()-start_time)/1000);
      Serial.println(" seconds");
      if (can_turn_off) {
        Serial.println("Minimum time elapsed, fridge can be turned off");
      }
    }else {
      Serial.println("System is idle");
    }
    lastUpdateTime = millis();
  
}

void get_temperatures() {
  sensors.requestTemperaturesByAddress(vatTempSensor);
  vatTemp = sensors.getTempC(vatTempSensor);
  sensors.requestTemperaturesByAddress(airTempSensor);
  airTemp = sensors.getTempC(airTempSensor);
  lastTempMeasurement = millis();
}

void loop() {
  // Safety measure: if no temperature measurement has been recieved
  // for more than 5min, terminate process
  if (millis() - lastTempMeasurement > error_interval) {
    #if (DEBUG) 
      Serial.println("Last temp measurement was more than 5min ago...");
      Serial.println(".....Kill everything.....");
    #endif
    //place both relays low?
  }
  adjust_setTemp();
  get_temperatures();
  //good idea to do call a safety function in loop, eg
  // 5 deg away from set point - kill everything

  //Display set and vat temperatures on lcd
  lcd.setCursor(0, 0);
  lcd.print("Set Temp: ");
  lcd.print(setTemp);
  lcd.setCursor(0, 1);
  lcd.print("Vat Temp: ");
  lcd.print(vatTemp);

  #if (DEBUG)
    if ((millis()-lastUpdateTime) > updateInterval) {
      status_update();
    }
  #endif

  //manage states - can't cool when heating or vice versa
  switch (state) {
    case STATE_IDLE:
      proc_idle();
      break;
    case STATE_HEAT:
      proc_heat();
      break;
    case STATE_COOL:
      proc_cool();
      break;
  }
}
