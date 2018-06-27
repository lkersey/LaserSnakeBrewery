#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RunningMedian.h>

// either debug OR production mode, as both use the serial 
#define DEBUG false
#define PRODUCTION true
#define ONE_WIRE_BUS 6
#define FRIDGE_RELAY 10
#define HEATER_RELAY 9

/*  tuning variables- tweak for more precise control
 *  Note: negative values mean below the set temperature.
 *  Be careful to ensure that variables do not overlap, to prevent
 *  competitve cycling
 */
float set_temp = 24; 
float heater_on_thresh = -0.3;
float heater_off_thresh = 0;
float fridge_on_thresh = 0.4;
float fridge_off_thresh = 0.1;

//set up non-tunable control variables
unsigned long RUN_THRESH = 120000; //2min in milliseconds, minimum time for heating/cooling elements to run
bool can_turn_off = true; //bool for short cycle timer
unsigned long start_time = 0; //variable for timing heating/cooling duration
unsigned long MAX_RUN_TIME = 14400000; //4 hours - maximum time for heating element to be on
unsigned long RELAX_TIME = 300000; //5min 

//system states
const int STATE_ERROR = -1;
const int STATE_RELAX = 0;
const int STATE_IDLE = 1;
const int STATE_COOL = 2;
const int STATE_HEAT = 3;
int state = STATE_IDLE; //initialise in idle

// set up 1-wire probes
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//DeviceAddress AIR_TEMP_SENSOR = {0x28, 0xFF, 0x7A, 0xF6, 0x82, 0x16, 0x03, 0x69};
//DeviceAddress VAT_TEMP_SENSOR = {0x28, 0xFF, 0xE3, 0x9C, 0x82, 0x16, 0x04, 0x25};
DeviceAddress AIR_TEMP_SENSOR = {0x28, 0xFF, 0x16, 0x8D, 0x87, 0x16, 0x03, 0x50}; //Test sensor A
DeviceAddress VAT_TEMP_SENSOR = {0x28, 0xFF, 0x0A, 0x2E, 0x68, 0x14, 0x04, 0xA6}; //Test sensor B

//set up temp measurement variables
bool air_probe_connected;
bool vat_probe_connected;
float air_temp;
float vat_temp;
unsigned long last_temp_request = 0;
bool waiting_for_conversion = false;
unsigned long CONVERSION_DELAY = 1000; //time allocated for temperature conversion
unsigned long MEAS_INTERVAL = 1000; //take temperature measurement every 1s
RunningMedian vatTempMedian = RunningMedian(60);
RunningMedian airTempMedian = RunningMedian(60);

unsigned long PUBLISH_PERIOD = 60000; //Publish values every minute
unsigned long last_publish = 0;

// Initialisations  
const int VAT_ID = 1;
const int AIR_ID = 2;

void setup() {
  Serial.begin(9600);
  
  // Set up temperature probes
  sensors.setResolution(AIR_TEMP_SENSOR, 11); //resolution of 0.125deg cels, 
  sensors.setResolution(VAT_TEMP_SENSOR, 11); //takes approx 375ms
  if (sensors.getResolution(VAT_TEMP_SENSOR) == 0) {
    vat_probe_connected = false;
    state = STATE_ERROR;
  } else {
    vat_probe_connected = true;
  }
  if (sensors.getResolution(AIR_TEMP_SENSOR) == 0) {
    air_probe_connected = false;
    state = STATE_ERROR;
  } else {
    air_probe_connected = true;
  }
  /* Setting the waitForConversion flag to false ensures that a tempurate request returns immediately
   *  without waiting for a temperature conversion. If setting the flag to false, be sure to wait 
   *  the appropriate amount of time before retrieving the measurement, to allow time for the conversion
   *  to take place.
   */
  sensors.setWaitForConversion(false);
  #if (DEBUG) 
    Serial.print("Vat sensor resolution: ");
    Serial.println(sensors.getResolution(VAT_TEMP_SENSOR), DEC);
    Serial.print("Air sensor resolution: ");
    Serial.println(sensors.getResolution(AIR_TEMP_SENSOR), DEC);
  #endif

  //initialise relays
  pinMode(FRIDGE_RELAY, OUTPUT);
  digitalWrite(FRIDGE_RELAY, HIGH);
  pinMode(HEATER_RELAY, OUTPUT);
  digitalWrite(HEATER_RELAY, HIGH);
}

void perform_action(String action) {
  //start timer so don't short cycle
  start_time = millis();
  if(action == "cool") {
    can_turn_off = false;
    #if (DEBUG)
      Serial.println("...Turning fridge on");
    #endif
    digitalWrite(FRIDGE_RELAY, LOW);
  } else if(action == "heat") {
    can_turn_off = false;
    #if (DEBUG)
      Serial.println("...Turning heating on");
    #endif
    digitalWrite(HEATER_RELAY, LOW);
  } else if (action == "disable") {
    #if (DEBUG) 
      Serial.println("...Switching elements off");
    #endif
    digitalWrite(HEATER_RELAY, HIGH);
    digitalWrite(FRIDGE_RELAY, HIGH);
  }
}

void proc_idle() {
  if (!vat_probe_connected || !air_probe_connected) {
    state = STATE_ERROR;
  }
  if (vat_temp - set_temp > fridge_on_thresh) {
    state = STATE_COOL; 
    perform_action("cool"); //activate cooling
  }else if (vat_temp - set_temp < heater_on_thresh) {
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
  if ((vat_temp - set_temp > heater_off_thresh) && can_turn_off) {
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
  } //fridge to turn off at set_temp + THRESH
  if ((vat_temp - set_temp < fridge_off_thresh) && can_turn_off) {
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
    if (can_turn_off) {
    #if (DEBUG)
      Serial.println("Exiting STATE_ERROR");
    #endif
      state = STATE_IDLE;
      //perform_action("disable");      
    }
  }
}

/* print status to Serial monitor, activated in debug mode only */
void status_update() {
  Serial.print("Vat temp:");
  Serial.print(vat_temp);
  Serial.print("\t");
  Serial.print("Air temp:");
  Serial.println(air_temp);
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
      if (!vat_probe_connected) {
        Serial.println("Vat probe not connected");
      }
      if (!air_probe_connected) {
        Serial.println("Air probe not connected");
      }
    }
}

void loop() {
  if (!waiting_for_conversion && (millis() - last_temp_request) > MEAS_INTERVAL) {
    air_probe_connected = sensors.requestTemperaturesByAddress(AIR_TEMP_SENSOR);
    vat_probe_connected = sensors.requestTemperaturesByAddress(VAT_TEMP_SENSOR);
    last_temp_request = millis();
    waiting_for_conversion = true;
  }
  if (waiting_for_conversion && millis()-last_temp_request > CONVERSION_DELAY) {
    vat_temp = sensors.getTempC(VAT_TEMP_SENSOR);
    air_temp = sensors.getTempC(AIR_TEMP_SENSOR);
    waiting_for_conversion = false;

    vatTempMedian.add(vat_temp);
    airTempMedian.add(air_temp);
    
     /* sent data to serial for collection from python script. 
     */
     if ( (millis() - last_publish) > PUBLISH_PERIOD) {
      #if (PRODUCTION)
        Serial.print("temperature_status;");
        Serial.print(vatTempMedian.getMedian());
        Serial.print(";");
        Serial.print(airTempMedian.getMedian());
        Serial.print(";");
        Serial.print(set_temp);
        Serial.print(";");
        Serial.println(state);
      #endif
      last_publish = millis();
     }
     
    //Send an update to the Serial monitor if DEBUG mode is on
    #if (DEBUG) 
      status_update();
    #endif
    
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
}
 
  

