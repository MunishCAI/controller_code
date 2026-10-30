/*
https://docs.google.com/document/d/1JQdvIDqlanoKpquYGsFicjRk7rIVaTCmWP4xZL8dpOw/edit?usp=sharing
*/

// Watchdog object
#include <Watchdog.h>
Watchdog watchdog;

// INPUT PIN'S
#define TIP_SENSOR1 32
#define TIP_SENSOR2 33
#define UV_SENSOR1  34
#define UV_SENSOR2  35
// 12V OUTPUT PIN'S
#define UV_LED1     2
#define UV_LED2     3
#define WHITE_LED1  4
#define WHITE_LED2  5
#define TOWER_LAMP1 6
#define TOWER_LAMP2 7
// 24V OUTPUT PIN'S
#define HAZARD_SIGNAL 8
#define CONVEYOR1     9
#define CONVEYOR2     10
//debouncing
#define DEBOUNCE_TIME 50

struct Sensor {
  int pin;
  int lastReading = HIGH;
  int stableState = HIGH;
  unsigned long debounceTime = DEBOUNCE_TIME;
  unsigned long lastChangeTime = 0;
    // Constructor
    Sensor(int sensorPin) {
        pin = sensorPin;
        debounceTime = DEBOUNCE_TIME;
        lastReading = HIGH;
        stableState = HIGH;
        lastChangeTime = 0;
    }
};

struct Timer {
  int delay;
  unsigned long previous_time;
};

Sensor  tip_sensor1 = {TIP_SENSOR1};       
Sensor  tip_sensor2 = {TIP_SENSOR2};       
Sensor  uv_sensor1  = {UV_SENSOR1 };     
Sensor  uv_sensor2  = {UV_SENSOR2 }; 

Timer alive_signal_timer = {1 , millis()};  // (DELAY REQUIRED , PREVIOUS TIMING)

bool debouncing(Sensor &sensor)
{
    int reading = digitalRead(sensor.pin);

    if (reading != sensor.lastReading)
    {
        sensor.lastChangeTime = millis();
        sensor.lastReading = reading;
    }

    if (millis() - sensor.lastChangeTime >= sensor.debounceTime)
    {
        if (reading != sensor.stableState)
        {
            sensor.stableState = reading;

            // Trigger once when the sensor becomes active
            if (sensor.stableState == LOW)
            {
                return true;
            }
        }
    }

    return false;
}

void set_initial_state()
{
  digitalWrite(UV_LED1      , HIGH);
  digitalWrite(UV_LED2      , HIGH);
  digitalWrite(WHITE_LED1   , HIGH);
  digitalWrite(WHITE_LED2   , HIGH);
  digitalWrite(TOWER_LAMP1  , LOW );
  digitalWrite(TOWER_LAMP2  , LOW );
  digitalWrite(HAZARD_SIGNAL, LOW );
  digitalWrite(CONVEYOR1    , LOW );
  digitalWrite(CONVEYOR2    , LOW );
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //
  pinMode(TIP_SENSOR1 , INPUT_PULLUP);
  pinMode(TIP_SENSOR2 , INPUT_PULLUP);
  pinMode(UV_SENSOR1  , INPUT_PULLUP);
  pinMode(UV_SENSOR2  , INPUT_PULLUP);

  pinMode(UV_LED1       , OUTPUT);
  pinMode(UV_LED2       , OUTPUT);
  pinMode(WHITE_LED1    , OUTPUT);
  pinMode(WHITE_LED2    , OUTPUT);
  pinMode(TOWER_LAMP1   , OUTPUT);
  pinMode(TOWER_LAMP2   , OUTPUT);
  pinMode(HAZARD_SIGNAL , OUTPUT);
  pinMode(CONVEYOR1     , OUTPUT);
  pinMode(CONVEYOR2     , OUTPUT);

  set_initial_state();   

}

void loop() {

  if(debouncing(tip_sensor1)){
    Serial.println("0");
  }
  if(debouncing(tip_sensor2)){
    Serial.println("1");
  }
  if(debouncing(uv_sensor1 )){
    Serial.println("2");
  }
  if(debouncing(uv_sensor2 )){
    Serial.println("3");
  }

  if (Serial.available() > 0)
  {
      char command = Serial.read();

      switch (command)
      {
          case 'A': digitalWrite(UV_LED1, HIGH); break;
          case 'a': digitalWrite(UV_LED1, LOW);  break;

          case 'B': digitalWrite(UV_LED2, HIGH); break;
          case 'b': digitalWrite(UV_LED2, LOW);  break;

          case 'C': digitalWrite(WHITE_LED1, HIGH); break;
          case 'c': digitalWrite(WHITE_LED1, LOW);  break;

          case 'D': digitalWrite(WHITE_LED2, HIGH); break;
          case 'd': digitalWrite(WHITE_LED2, LOW);  break;

          case 'E': digitalWrite(TOWER_LAMP1, HIGH); break;
          case 'e': digitalWrite(TOWER_LAMP1, LOW);  break;

          case 'F': digitalWrite(TOWER_LAMP2, HIGH); break;
          case 'f': digitalWrite(TOWER_LAMP2, LOW);  break;

          case 'G': digitalWrite(HAZARD_SIGNAL, HIGH); break;
          case 'g': digitalWrite(HAZARD_SIGNAL, LOW);  break;

          case 'H': digitalWrite(CONVEYOR1, HIGH); break;
          case 'h': digitalWrite(CONVEYOR1, LOW);  break;

          case 'I': digitalWrite(CONVEYOR2, HIGH); break;
          case 'i': digitalWrite(CONVEYOR2, LOW);  break;
      }
  }
  
  if(millis() - alive_signal_timer.previous_time >=alive_signal_timer.delay)
  {
    Serial.println("L");
    alive_signal_timer.previous_time = millis();
  }

}
