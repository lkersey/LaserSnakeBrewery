#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#define DEBUG true
#define PRODUCTION false

#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/* Address of 1-wire sensors can be found by following the tutorial at 
 * http://henrysch.capnfatz.com/henrys-bench/arduino-temperature-measurements/ds18b20-arduino-user-manual-introduction-and-contents/ds18b20-user-manual-part-2-getting-the-device-address/
 */
DeviceAddress AIR_TEMP_SENSOR = {0x28, 0xFF, 0x7A, 0xF6, 0x82, 0x16, 0x03, 0x69};
DeviceAddress VAT_TEMP_SENSOR = {0x28, 0xFF, 0xE3, 0x9C, 0x82, 0x16, 0x04, 0x25};
bool air_probe_connected;
bool vat_probe_connected;
float air_temp;
float vat_temp;
unsigned long temp_request = 0;
bool waiting_for_conversion = false;
unsigned long REQUEST_DURATION = 1000;
unsigned long last_temp_measurement = 0;
const float THRESH = 0.5;
unsigned long MEAS_INTERVAL = 5000; //take temperature measurement every 10s
//short cycle timer
unsigned long RUN_THRESH = 120000; //2min in milliseconds, minimum time for heating/cooling elements to run
//bool for short cycle
bool can_turn_off = true;
// desired temperature, default is 20 deg C
float set_temp = 20; 
unsigned long start_time = 0; //variable for timing heating/cooling duration

// Initialisations for myController. 
const int VAT_ID = 1;
const int AIR_ID = 2;
String vat_payload;
String air_payload;

//system states
const int STATE_ERROR = -1;
const int STATE_IDLE = 1;
const int STATE_COOL = 2;
const int STATE_HEAT = 3;
//initialise in idle mode
int state = STATE_IDLE; 

//Buttons for adjusting set temperature
const int INC_BUTTON_PIN = 8;
const int DEC_BUTTON_PIN = 7;
unsigned long debounce_start = 0;  // the last time the output pin was toggled
unsigned long DEBOUNCE_DELAY = 100;  // the debounce time; increase if the output flickers
const float TEMP_INCREMENT = 0.1;

//LCD display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  /* Present sensors to MyController. Detailed presentation instructions can be
  *  found here: https://www.mysensors.org/download/serial_api_20
  */
  #if (PRODUCTION)
    Serial.println("1;1;0;0;6");
    Serial.println("1;2;0;0;6");
  #endif
  
  //set up buttons
  pinMode(INC_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(INC_BUTTON_PIN, HIGH);
  pinMode(DEC_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(DEC_BUTTON_PIN, HIGH);
  
  // Set up temperature probes
  sensors.setResolution(AIR_TEMP_SENSOR, 11); //resolution of 0.125deg cels, 
  sensors.setResolution(VAT_TEMP_SENSOR, 11); //takes approx 375ms
  sensors.setWaitForConversion(false);
  #if (DEBUG) 
    Serial.print("Vat sensor resolution: ");
    Serial.println(sensors.getResolution(VAT_TEMP_SENSOR), DEC);
    Serial.print("Air sensor resolution: ");
    Serial.println(sensors.getResolution(AIR_TEMP_SENSOR), DEC);
  #endif
  
  // initialise lcd
  lcd.begin(16,2);
}

void adjust_set_temp(){
  if(digitalRead(INC_BUTTON_PIN) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > DEBOUNCE_DELAY) {
      set_temp += TEMP_INCREMENT;
      debounce_start = 0; //reset debounce timer
    }
  }
  if(digitalRead(DEC_BUTTON_PIN) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > DEBOUNCE_DELAY) {
      set_temp -= TEMP_INCREMENT;
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
    //write heater relay pin high here (if active high)
  }
}

void proc_idle() {
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  if (vat_temp - set_temp > THRESH) {
    state = STATE_COOL; 
    activate_relay("cool"); //activate cooling
  } else if (set_temp - vat_temp > THRESH) {
    state = STATE_HEAT;
    activate_relay("heat");
  } else {
  //write digital pins low here for relays
  }
}

void proc_heat() {
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  if ((vat_temp > set_temp) && (can_turn_off)) { 
    #if (DEBUG)
      Serial.println("...Switching heat off...");
    #endif
    state = STATE_IDLE;
  }
}

void proc_cool() {
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  if ((vat_temp < set_temp) && (can_turn_off)) {
    #if (DEBUG)
      Serial.println("...Switching fridge off...");
    #endif
    state = STATE_IDLE;
  }
}

void error_handler() {
  if (millis() - start_time > RUN_THRESH) {
    can_turn_off = true;
    // set both relays low here 
  }
  // Determine the cause of the error and display to lcd


  // if probe comes back 
  if (vat_probe_connected && air_probe_connected) {
    #if (DEBUG)
      Serial.println("Exiting STATE_ERROR");
    #endif
    state = STATE_IDLE;
    lcd.clear();
  }
}

/* print status to Serial monitor, activated in debug mode only */
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
    }else if (state == STATE_IDLE) {
      Serial.println("System is idle");
    } else if (state == STATE_ERROR) {
      Serial.println("ERROR");
    }
}

void update_lcd() {
  switch(state) {
    case STATE_ERROR:
      if (!vat_probe_connected) {
        lcd.setCursor(0,0);
        //lcd.clear();
        lcd.print("Error: vat probe");
      } 
      if (!air_probe_connected) {
        lcd.setCursor(0,1);
        //lcd.clear();
        lcd.print("Error: air probe");
      }
      break;
      default:
        lcd.setCursor(0, 0);
        lcd.print("Set Temp: ");
        lcd.print(set_temp);
        lcd.setCursor(0, 1);
        lcd.print("Vat Temp: ");
        lcd.print(vat_temp);
        break;
    }
}

void loop() {
  adjust_set_temp();
  if (!waiting_for_conversion) {
    air_probe_connected = sensors.requestTemperaturesByAddress(VAT_TEMP_SENSOR);
    vat_probe_connected = sensors.requestTemperaturesByAddress(AIR_TEMP_SENSOR);

    temp_request = millis();
    last_temp_measurement = millis();
    waiting_for_conversion = true;
  }
  if (millis()-temp_request>750) {
    vat_temp = sensors.getTempC(VAT_TEMP_SENSOR);
    air_temp = sensors.getTempC(AIR_TEMP_SENSOR);
    waiting_for_conversion = false;
  }
  update_lcd();
  //if (millis() - last_temp_measurement > MEAS_INTERVAL) {
   // get_temperatures();

    //Display vat temperature on lcd

    
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
      case STATE_ERROR:
        error_handler();
        break;    
    }

    //Send an update to the lcd if DEBUG mode is on
    #if (DEBUG) 
      status_update();
    #endif
    
 /* sent data to myController. Detailed instructions can be
 *  found here: https://www.mysensors.org/download/serial_api_20
 */
    vat_payload = vat_temp;
    air_payload = air_temp;
    #if (PRODUCTION)
      Serial.println("1;1;1;0;0;" + vat_payload);
      Serial.println("1;2;1;0;0;" + air_payload);
    #endif
   // }
  }
  

