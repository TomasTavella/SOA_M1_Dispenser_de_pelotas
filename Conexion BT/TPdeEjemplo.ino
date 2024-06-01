#include <Servo.h>
#include <Wire.h>
#include <rgb_lcd.h>
#include <SoftwareSerial.h>

//---------------------------
// DISPLAY
//---------------------------

#define DISPLAY_ADDRESS 32
#define DISPLAY_MAX_WIDTH 16
#define DISPLAY_MAX_HEIGHT 2
#define DISPLAY_WIDTH_0 0
#define DISPLAY_WIDTH_1 1
#define DISPLAY_WIDTH_2 2
#define DISPLAY_WIDTH_3 3
#define DISPLAY_WIDTH_4 4
#define DISPLAY_HEIGHT_0 0
#define DISPLAY_HEIGHT_1 1
rgb_lcd LCD;
void display_init();
void display_print(int width, int height, String msg);

//---------------------------
// STATES
//---------------------------

#define INIT 0
#define CLEAN 1
#define CAT_INSIDE 2
#define CAT_OUTSIDE 3
#define SLIGHTLY_DIRTY 4
#define MEDIUMLY_DIRTY 5
#define HIGHLY_DIRTY 6
#define CLEANING  7
#define CONTINUE_STATE 8 


String states[] = {"INIT", "CLEAN", "CAT_INSIDE", "CAT_OUTSIDE", "DIRTY_SLIGHTLY", "DIRTY_MEDIUMLY", "DIRTY_HIGHLY", "CLEANING"};

//---------------------------
// RGB HEADERS
//---------------------------

#define GREEN 1
#define YELLOW 2
#define ORANGE 3
#define RED 4
#define BLUE 5
#define RGB_255 255
#define RGB_0 0
#define RGB_100 100
#define PIN_LED_GREEN 5
#define PIN_LED_BLUE 6
#define PIN_LED_RED 7
void rgb_init();


//---------------------------
// EVENTS
//---------------------------

#define ENTRANCE_DETECTED   0
#define EXIT_DETECTED       1
#define NO_DIRTINESS        2
#define LOW_DIRTINESS       3
#define MID_DIRTINESS       4
#define HIGH_DIRTINESS      5
#define BUTTON_1_ACTIVATED  6
#define BUTTON_2_ACTIVATED  7
#define CONTINUE            8
#define GET_STATE           9
#define GET_HUMIDITY        10

String events[] = {"ENTRANCE_DETECTED", "EXIT_DETECTED", "NO_DIRTINESS", "LOW_DIRTINESS",
                    "MID_DIRTINESS", "HIGH_DIRTINESS", "BUTTON_1_ACTIVATED", "BUTTON_2_ACTIVATED", "CONTINUE"};

//---------------------------
// SERVO HEADER
//---------------------------

#define SERVO_PIN 9
#define SERVO_OPEN 0
#define SERVO_CLOSE 180
Servo Servomotor;
void servo_init();

//---------------------------
// MOISTURE HEADER
//---------------------------

#define MAX_MOISTURE_VALUE 539.0
#define POOP 2
#define LOW_DIRTINESS_MIN_VAL 1
#define MID_DIRTINESS_MIN_VAL 4
#define HIGH_DIRTINESS_MIN_VAL 6
#define MOISTURE_MAX_POINTS 10.0
#define INITIAL_MOISURE 0
#define INITIAL_DIRTINESS 0
#define MOISTURE_MSG_SIZE 50
char moisture_msg[MOISTURE_MSG_SIZE];
const int moisture_sensor = A0;
float prev_moisture = 0.0;
float dirtiness_level = 0;
bool verify_moisture();
float moisture_get_points(float moisture);

//---------------------------
// DISTANCE SENSOR HEADERS
//---------------------------

#define DISTANCE_INSIDE  0
#define DISTANCE_OUTSIDE 1
#define DISTANCE_SENSOR 12
#define MIN_DISTANCE 50
#define SPEED_OF_SOUND 0.01723
#define PULSE_WIDTH_2 2
#define PULSE_WIDTH_10 10
int distance_state;
int dist;
long time_to_object;
void distance_sensor_init();
bool verify_distance();
long distance_read(int trigger_pin, int echo_pin);

//---------------------------
// BUTTON HEADER
//---------------------------

#define PRESSED_ONCE    0
#define PRESSED_TWICE   1
#define NOT_PRESSED     2
#define PIN_BUTTON 4
int button_state;
int prev_button_state = LOW;
int button_current_state = NOT_PRESSED;
void button_init();
bool verify_button();


//---------------------------
// BLUETOOTH HEADER
//---------------------------
#define PIN_BLUETOOTH_RX 10 // Arduino RX
#define PIN_BLUETOOTH_TX 11 // Arduino TX
#define BLUETOOTH_BPS 9600
#define BLUETOOTH_MSG_STOP 'S'
#define BLUETOOTH_MSG_CLEAN 'C'
#define BLUETOOTH_MSG_GET_STATE 's'
#define BLUETOOTH_MSG_GET_HUMIDITY 'h'
char bt_msg;
void bluetooth_init();
bool verify_bluetooth();
void bluetooth_send_humidity();
void bluetooth_send_state();
SoftwareSerial Bluetooth(PIN_BLUETOOTH_RX, PIN_BLUETOOTH_TX);

//---------------------------
// Global Variables
//---------------------------

int state;
int event;
bool exited = false;

void setup()
{
  display_init();
  distance_sensor_init();
  rgb_init();
  button_init();
  servo_init();
  bluetooth_init();

  state = INIT;
}

void loop()
{
  get_event();
  state_machine();
}

void state_machine() { 
  switch(state)
  {
    case INIT:
      state = CLEAN;
      LCD.clear();
      display_print(DISPLAY_WIDTH_0, DISPLAY_HEIGHT_0, "INICIANDO...");
      LCD.clear();
      display_print(DISPLAY_WIDTH_1, DISPLAY_HEIGHT_0, "ESTADO: LIMPIO");
    break;
    case CLEAN:
      switch(event)
      {
        case ENTRANCE_DETECTED:
          state = CAT_INSIDE;
          LCD.clear();
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_0, "GATO DENTRO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "NO MOLESTAR");
        break;
        case BUTTON_1_ACTIVATED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_4, DISPLAY_HEIGHT_0, "LIMPIANDO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "SENSORES: OFF");
          changeLED(BLUE);
          state = CLEANING;
          Servomotor.write(SERVO_CLOSE);
        break;
        case CONTINUE:
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        default:
        break;
      }
    break;

    case CAT_INSIDE:
      switch(event)
      {
        case EXIT_DETECTED:
          display_print(DISPLAY_WIDTH_0, DISPLAY_HEIGHT_0, "CALCULANDO .....");
          display_print(DISPLAY_WIDTH_0, DISPLAY_HEIGHT_1, "Espere Por Favor");
          exited = true;            
          state = CAT_OUTSIDE;
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        default:
        break;
      }
    break;

    case CAT_OUTSIDE:
      switch(event)
      {
        case NO_DIRTINESS:
          LCD.clear();
          display_print(DISPLAY_WIDTH_1, DISPLAY_HEIGHT_0, "ESTADO: LIMPIO");
          state = CLEAN;
        break;
        case LOW_DIRTINESS:
          LCD.clear();
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_0, "ESTADO: LEVE");
          state = SLIGHTLY_DIRTY;
          changeLED(YELLOW);
        break;
        case MID_DIRTINESS:
          LCD.clear();
          display_print(DISPLAY_WIDTH_1, DISPLAY_HEIGHT_0, "ESTADO: MEDIO");
          state = MEDIUMLY_DIRTY;
          changeLED(ORANGE);
        break;
        case HIGH_DIRTINESS:
          LCD.clear();
          display_print(DISPLAY_WIDTH_0, DISPLAY_HEIGHT_0, "ESTADO: CRITICO");
          state = HIGHLY_DIRTY;
          changeLED(RED);
          Servomotor.write(SERVO_CLOSE);
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        default:
        break;
      }
    break;

    case SLIGHTLY_DIRTY: 
      switch(event)
      {
        case ENTRANCE_DETECTED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_0, "GATO DENTRO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "NO MOLESTAR");
          state = CAT_INSIDE;
        break;
        case BUTTON_1_ACTIVATED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_0, "GATO DENTRO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "NO MOLESTAR");
          changeLED(BLUE);
          state = CLEANING;
          Servomotor.write(SERVO_CLOSE);
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        case CONTINUE:
        break;
        default:
        break;
      }    
    break;

    case MEDIUMLY_DIRTY:
      switch(event)
      {
        case ENTRANCE_DETECTED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_0, "GATO DENTRO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "NO MOLESTAR");
          state = CAT_INSIDE;
        break;
        case BUTTON_1_ACTIVATED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_4, DISPLAY_HEIGHT_0, "LIMPIANDO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "Sensores: OFF");
          changeLED(BLUE);
          state = CLEANING;
          Servomotor.write(SERVO_CLOSE);
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        case CONTINUE:
        break;
        default:
        break;
      }    
    break;

    case HIGHLY_DIRTY:
      switch(event)
      {
        case BUTTON_1_ACTIVATED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_4, DISPLAY_HEIGHT_0, "LIMPIANDO");
          display_print(DISPLAY_WIDTH_2, DISPLAY_HEIGHT_1, "Sensores: OFF");
          changeLED(BLUE);
          state = CLEANING;
          Servomotor.write(SERVO_CLOSE);
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        case CONTINUE:
        break;
        default:
        break;
      }    
    break;

    case CLEANING:
      switch(event)
      {
        case BUTTON_2_ACTIVATED:
          LCD.clear();
          display_print(DISPLAY_WIDTH_1, DISPLAY_HEIGHT_0, "ESTADO: LIMPIO");
          state = CLEAN;
          prev_moisture = INITIAL_MOISURE;
          dirtiness_level = INITIAL_DIRTINESS;
          changeLED(GREEN);
          Servomotor.write(SERVO_OPEN);
        break;
        case GET_STATE:
            bluetooth_send_state();
            break;
        case GET_HUMIDITY:
            bluetooth_send_humidity();
            break;
        case CONTINUE:
        break;
        default:
        break;
      }
    break;

    default:
    break;
  }
}

void get_event()
{
  if(verify_distance() == true || verify_button() == true || verify_bluetooth() == true  || verify_moisture() == true)
    return;
  event = CONTINUE;
}

bool verify_bluetooth()
{
  if(Bluetooth.available())
  {
    bt_msg = Bluetooth.read();
    if(bt_msg == BLUETOOTH_MSG_STOP)
    {
        event = BUTTON_1_ACTIVATED;
        return true;
    }
    if(bt_msg == BLUETOOTH_MSG_CLEAN)
    {
        event = BUTTON_2_ACTIVATED;
        return true;
    }

    if(bt_msg == BLUETOOTH_MSG_GET_STATE)
    {
        event = GET_STATE;
        return true;
    }

    if (bt_msg == BLUETOOTH_MSG_GET_HUMIDITY)
    {
        event = GET_HUMIDITY;
        return true;
    }

    return false;
  }

  return false;
}

bool verify_distance()
{
  time_to_object = distance_read(DISTANCE_SENSOR, DISTANCE_SENSOR);
  dist = time_to_object * SPEED_OF_SOUND;

  switch(distance_state)
  {
    case DISTANCE_OUTSIDE:
      if(dist < MIN_DISTANCE)
      {
        distance_state = DISTANCE_INSIDE;
        event = ENTRANCE_DETECTED;
        return true;
      }
      return false;
    break;

    case DISTANCE_INSIDE:
      if(dist >= MIN_DISTANCE)
      {
        distance_state = DISTANCE_OUTSIDE;
        event = EXIT_DETECTED;
        return true;
      }
      return false;
    break;
  }
}

bool verify_button()
{
  button_state = digitalRead(PIN_BUTTON);
  if (button_state != prev_button_state) 
  {
    if (button_state == HIGH)
    {
      switch (button_current_state) 
      {
        case PRESSED_ONCE:
          button_current_state = PRESSED_TWICE;
          event = BUTTON_2_ACTIVATED;
          prev_button_state = button_state;
          return true;
        break;

        case PRESSED_TWICE:
          button_current_state = PRESSED_ONCE;
          event = BUTTON_1_ACTIVATED;
          prev_button_state = button_state;
          return true;
        break;
        
        case NOT_PRESSED:
          button_current_state = PRESSED_ONCE;
          event = BUTTON_1_ACTIVATED;
          prev_button_state = button_state;
          return true;
        break;
      }
    }
  }

  prev_button_state = button_state;
  return false;
}

bool verify_moisture()
{ 
  float moisture = analogRead(moisture_sensor);
  float moisture_val_out_of_ten;

  if(exited == true)
  {
    if(moisture > prev_moisture)
    {
      moisture_val_out_of_ten = moisture_get_points(moisture);
      dirtiness_level += moisture_val_out_of_ten;
      prev_moisture = moisture;
    }
    else
    {
      dirtiness_level += POOP;
    }

    if(dirtiness_level < LOW_DIRTINESS_MIN_VAL)
          event = NO_DIRTINESS;
    else if (dirtiness_level < MID_DIRTINESS_MIN_VAL)
          event = LOW_DIRTINESS;
    else if (dirtiness_level < HIGH_DIRTINESS_MIN_VAL)
          event = MID_DIRTINESS;
    else
          event = HIGH_DIRTINESS;

    exited = false;
    return true;
  }

  return false;
}

void changeLED(int color)
{
  switch(color){
    case GREEN:
      analogWrite(PIN_LED_GREEN, RGB_255);
      analogWrite(PIN_LED_RED, RGB_0);
      analogWrite(PIN_LED_BLUE, RGB_0);
    break;
    case YELLOW:
      analogWrite(PIN_LED_GREEN, RGB_255);
      analogWrite(PIN_LED_RED, RGB_255);
      analogWrite(PIN_LED_BLUE, RGB_0);
    break;
    case ORANGE:
      analogWrite(PIN_LED_GREEN, RGB_100);
      analogWrite(PIN_LED_RED, RGB_255); 
      analogWrite(PIN_LED_BLUE, RGB_0);
    break;
    case RED:
      analogWrite(PIN_LED_GREEN, RGB_0);
      analogWrite(PIN_LED_RED, RGB_255);
      analogWrite(PIN_LED_BLUE, RGB_0);
    break;
    case BLUE:
      analogWrite(PIN_LED_GREEN, RGB_0);
      analogWrite(PIN_LED_RED, RGB_0);
      analogWrite(PIN_LED_BLUE, RGB_255);
    break;
    default:
    break;
  }
}



//---------------------------
// SERVO IMPLEMENTATION
//---------------------------

void servo_init()
{
  Servomotor.attach(SERVO_PIN);
  Servomotor.write(SERVO_OPEN);
}

//---------------------------
// DISPLAY IMPLEMENTATION
//---------------------------

void display_init()
{
  LCD.begin(DISPLAY_MAX_WIDTH, DISPLAY_MAX_HEIGHT);
}

void display_print(int width, int height, String msg)
{
  LCD.setCursor(width, height);
  LCD.print(msg);
}


//---------------------------
// MOISTURE SENSOR IMPLEMENTATION
//---------------------------

float moisture_get_points(float moisture)
{
  return (moisture / MAX_MOISTURE_VALUE) * MOISTURE_MAX_POINTS;
}

//---------------------------
// DISTANCE SENSOR IMPLEMENTATION
//---------------------------

void distance_sensor_init()
{
  distance_state = DISTANCE_OUTSIDE;
}

long distance_read(int trigger_pin, int echo_pin)
{
  pinMode(trigger_pin, OUTPUT);

  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(PULSE_WIDTH_2);

  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(PULSE_WIDTH_10);
  
  digitalWrite(trigger_pin, LOW);
  pinMode(echo_pin, INPUT);

  return pulseIn(echo_pin, HIGH);
}


//---------------------------
// RGB IMPLEMENTATION
//---------------------------

void rgb_init()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  changeLED(GREEN);
}

//---------------------------
// BUTTON IMPLEMENTATION
//---------------------------

void button_init()
{
  pinMode(PIN_BUTTON, INPUT);
}

//---------------------------
// BLUETOOTH IMPLEMENTATION
//---------------------------
void bluetooth_init()
{
    Bluetooth.begin(BLUETOOTH_BPS);
}

void bluetooth_send_humidity()
{
  snprintf(moisture_msg, MOISTURE_MSG_SIZE, "InteLitter Humidity: %d\n", analogRead(moisture_sensor));
  Bluetooth.write(moisture_msg);
}

void bluetooth_send_state()
{
    switch (state)
    {
    case INIT:
        Bluetooth.write("InteLitter Estado: INICIO\n");
        break;
    case CLEAN:
        Bluetooth.write("InteLitter Estado: LIMPIO\n");
        break;
    case CAT_INSIDE:
        Bluetooth.write("InteLitter Estado: GATO DENTRO\n");
        break;
    case CAT_OUTSIDE:
        Bluetooth.write("InteLitter Estado: GATO FUERA\n");
        break;
    case SLIGHTLY_DIRTY:
        Bluetooth.write("InteLitter Estado: POCO SUCIA\n");
        break;
    case MEDIUMLY_DIRTY:
        Bluetooth.write("InteLitter Estado: MEDIANAMENTE SUCIA\n");
        break;
    case HIGHLY_DIRTY:
        Bluetooth.write("InteLitter Estado: SUCIA\n");
        break;
    case CLEANING:
        Bluetooth.write("InteLitter Estado: LIMPIANDO\n");
        break;
    case CONTINUE_STATE:
        Bluetooth.write("InteLitter Estado: CONTINUA\n");
        break;
    default:
        break;
    }
}