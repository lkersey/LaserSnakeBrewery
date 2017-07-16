#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#define DEBUG true
#define PRODUCTION false
#define ONE_WIRE_BUS 13
#define FRIDGE_RELAY 9
#define HEATER_RELAY 10

// set up 1-wire probes
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/* Address of 1-wire sensors can be found by following the tutorial at 
 * http://henrysch.capnfatz.com/henrys-bench/arduino-temperature-measurements/ds18b20-arduino-user-manual-introduction-and-contents/ds18b20-user-manual-part-2-getting-the-device-address/
 */
DeviceAddress AIR_TEMP_SENSOR = {0x28, 0xFF, 0x7A, 0xF6, 0x82, 0x16, 0x03, 0x69};
DeviceAddress VAT_TEMP_SENSOR = {0x28, 0xFF, 0xE3, 0x9C, 0x82, 0x16, 0x04, 0x25};

//set up temp measurement variables
bool air_probe_connected;
bool vat_probe_connected;
float air_temp;
float vat_temp;
unsigned long last_temp_request = 0;
//unsigned long last_temp_measurement = 0;
bool waiting_for_conversion = false;
unsigned long CONVERSION_DELAY = 1000; //time allocated for temperature conversion
unsigned long MEAS_INTERVAL = 1000; //take temperature measurement every 1s

//set up control variables
const float THRESH = 0.5;
unsigned long RUN_THRESH = 120000; //2min in milliseconds, minimum time for heating/cooling elements to run
bool can_turn_off = true; //bool for short cycle timer
unsigned long MAX_RUN_TIME = 14400000; //4 hours - maximum time for heating element to be on
//unsigned long MAX_RUN_TIME = 2000;
unsigned long RELAX_TIME = 300000; //5min 
float set_temp = 20; 
unsigned long start_time = 0; //variable for timing heating/cooling duration
//system states
const int STATE_ERROR = -1;
const int STATE_RELAX = 0;
const int STATE_IDLE = 1;
const int STATE_COOL = 2;
const int STATE_HEAT = 3;
int state = STATE_IDLE; //initialise in idle

// Initialisations for myController. 
const int VAT_ID = 1;
const int AIR_ID = 2;
String vat_payload;
String air_payload;


//Buttons for adjusting set temperature
const int INC_BUTTON_PIN = 8;
const int DEC_BUTTON_PIN = 7;
unsigned long debounce_start = 0;  // the last time the output pin was toggled
unsigned long DEBOUNCE_DELAY = 100;  // the debounce time; increase if the output flickers
const float TEMP_INCREMENT = 0.1;

//LCD display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//unsigned long LCD_REFRESH = 300000; //refresh lcd every 5min
//unsigned long last_lcd_refresh = 0;

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

  //initialise relays
  pinMode(FRIDGE_RELAY, OUTPUT);
  pinMode(HEATER_RELAY, OUTPUT);
  
  // initialise lcd
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Set Temp: ");
  lcd.setCursor(0,1);
  lcd.print("Vat temp: ");
}

void adjust_set_temp(){
  if(digitalRead(INC_BUTTON_PIN) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > DEBOUNCE_DELAY) {
      set_temp += TEMP_INCREMENT;
      debounce_start = 0; //reset debounce timer
      update_lcd(); 
    }
  }
  if(digitalRead(DEC_BUTTON_PIN) == LOW){
    if (debounce_start == 0) {
      debounce_start = millis();
    }
    if ( (millis() - debounce_start) > DEBOUNCE_DELAY) {
      set_temp -= TEMP_INCREMENT;
      debounce_start = 0; //reset debounce timer
      update_lcd();
    }
  }
}

void perform_action(String action) {
  //start timer so don't short cycle
  start_time = millis();
  if(action == "cool") {
    can_turn_off = false;
    #if (DEBUG)
      Serial.println("...Turning fridge on");
    #endif
    // pull fridge relay high
    digitalWrite(FRIDGE_RELAY, HIGH);
  } else if(action == "heat") {
    can_turn_off = false;
    #if (DEBUG)
      Serial.println("...Turning heating on");
    #endif
    // pull heater relay high
    digitalWrite(HEATER_RELAY, HIGH);
  }else if (action == "relax") {
    #if (DEBUG) 
      Serial.println("...Switching elements off");
    #endif
    can_turn_off = true;
    // write relay pins low here
    digitalWrite(HEATER_RELAY, LOW);
  } else if (action == "disable") {
    #if (DEBUG) 
      Serial.println("...Switching elements off");
    #endif
    digitalWrite(HEATER_RELAY, LOW);
    digitalWrite(FRIDGE_RELAY, LOW);
  }
}

void proc_idle() {
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  if (vat_temp - set_temp > THRESH) {
    state = STATE_COOL; 
    perform_action("cool"); //activate cooling
  } else if (set_temp - vat_temp > THRESH) {
    state = STATE_HEAT;
    perform_action("heat");
  }
}

void proc_heat() {
  // check if probes are connected
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  // check if minimum runtime has elapsed
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  // check if conditions are right to turn heater off 
  if ((vat_temp > set_temp) && (can_turn_off)) { 
    state = STATE_IDLE;
    perform_action("disable");
  }
  // check if heater has been on for too long
  if ((millis() - start_time) > MAX_RUN_TIME) {
    #if (DEBUG) 
      Serial.println("Maximum heat time exceeded, entering STATE_RELAX");
    #endif
    state = STATE_RELAX;
    perform_action("disable");
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
    state = STATE_IDLE;
    perform_action("disable");
  }
}

void proc_relax() {
  if (millis() - start_time > RELAX_TIME) {
    #if (DEBUG) 
      Serial.println("Relax time exceeded, entering STATE_IDLE");
    #endif
    state = STATE_IDLE;
  }
}

void error_handler() {
  if (millis() - start_time > RUN_THRESH) {
    can_turn_off = true;
    perform_action("disable"); 
  }
  // if probe comes back 
  if (vat_probe_connected && air_probe_connected) {
    #if (DEBUG)
      Serial.println("Exiting STATE_ERROR");
    #endif
    if (can_turn_off) {
      state = STATE_IDLE;
      //perform_action("disable");      
    }
    reset_lcd();
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

void reset_lcd() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set Temp: ");
  lcd.setCursor(0,1);
  lcd.print("Vat temp: ");
}

void update_lcd() {
  switch(state) {
    case STATE_ERROR:
      if (!vat_probe_connected) {
        lcd.setCursor(0,0);
        //lcd.clear();
        lcd.print("Error: vat probe");
      } else if (vat_probe_connected) {
        lcd.setCursor(0,0);
        lcd.print("                ");
      }
      if (!air_probe_connected) {
        lcd.setCursor(0,1);
        //lcd.clear();
        lcd.print("Error: air probe");
      } else if (air_probe_connected) {
        lcd.setCursor(0,1);
        lcd.print("                ");
      }
      break;
      default:
        lcd.setCursor(10, 0);
        lcd.print("    ");
        lcd.setCursor(10,0);
        lcd.print(set_temp);
        lcd.setCursor(10, 1);
        lcd.print("    ");
        lcd.setCursor(10,1);
        lcd.print(vat_temp);
        break;
    }
}

void loop() {
  adjust_set_temp();
  //update_lcd();
  
  if (!waiting_for_conversion && (millis() - last_temp_request) > MEAS_INTERVAL) {
    air_probe_connected = sensors.requestTemperaturesByAddress(AIR_TEMP_SENSOR);
    vat_probe_connected = sensors.requestTemperaturesByAddress(VAT_TEMP_SENSOR);
    //Serial.println("Entering temperaure request");
    last_temp_request = millis();
    waiting_for_conversion = true;
  }
  if (waiting_for_conversion && millis()-last_temp_request > CONVERSION_DELAY) {
    vat_temp = sensors.getTempC(VAT_TEMP_SENSOR);
    air_temp = sensors.getTempC(AIR_TEMP_SENSOR);
    waiting_for_conversion = false;
    update_lcd();
   /* sent data to myController. Detailed instructions can be
   *  found here: https://www.mysensors.org/download/serial_api_20
   */
    #if (PRODUCTION)
      vat_payload = vat_temp;
      air_payload = air_temp;
      Serial.println("1;1;1;0;0;" + vat_payload);
      Serial.println("1;2;1;0;0;" + air_payload);
    #endif
    //Send an update to the Serial monitor if DEBUG mode is on
    #if (DEBUG) 
      status_update();
    #endif
  }
    // Manage states- can't heat while cooling and vice versa
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
      case STATE_RELAX:
        proc_relax();
        break;
      case STATE_ERROR:
        error_handler();
        break;    
    } 
 }
  

