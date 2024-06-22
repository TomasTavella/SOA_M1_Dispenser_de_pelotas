#include <Servo.h>
#include <SoftwareSerial.h>

// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define VALUE_CONTINUE -1


// ------------------------------------------------
// Temporizadores
// ------------------------------------------------
#define TIME_EVENT_MILIS 500
#define TIME_WAIT_MILIS 3000
#define TIME_SERVO_MILIS 700

//---------------------------
// SERVO INIT
//---------------------------
#define SERVO_PIN 9
#define SERVO_OPEN 180
#define SERVO_CLOSE 90
Servo Servomotor;

//---------------------------
// DISTANCE SENSOR INIT
//---------------------------
#define DISTANCE_SENSOR_PINECHO_DOG 12
#define DISTANCE_SENSOR_PINTRIG_DOG 13
#define DISTANCE_SENSOR_PINECHO_BALL 10
#define DISTANCE_SENSOR_PINTRIG_BALL 11
#define DELAY_PULSE_2 2
#define DELAY_PULSE_10 10
#define SPEED_OF_SOUND_CM_PER_MICROSECOND 0.01723
#define UMBRAL_DISTANCE_DOG 20
#define UMBRAL_DISTANCE_BALL 5
//long distance_read(int distance_pin);
//---------------------------
// BUTTON INIT
//---------------------------
#define PIN_BUTTON 8

//---------------------------
// RGB INIT
//---------------------------
#define PIN_LED_GREEN 7
#define PIN_LED_BLUE 6
#define PIN_LED_RED 5

#define NONE 0
#define GREEN 1
#define RED 2
#define YELLOW 3

// ------------------------------------------------
// Bluetooth
// ------------------------------------------------
//#define BLUETOOTH_BUTTON 1
char bt_msg;
const char BLUETOOTH_BUTTON = '0';
const char BLUETOOTH_STATE_REQUEST = '1';
void bluetooth_send_state();
void send_empty_message();
bool verify_bluetooth();
SoftwareSerial bluetooth(3, 4);


// ------------------------------------------------
// states del embebido
// ------------------------------------------------
enum state_e
{
    STATE_CHECKING,
    STATE_READY,
    STATE_DOG_DETECTED,
    STATE_DROP_BALL,
    STATE_END_OF_SERVICE    //agrego estado para final de servicio (espera a que el perro se aleje)
};

// ------------------------------------------------
// events posibles
// ------------------------------------------------
enum event_e
{
    EVENT_NOT_EMPTY,
    EVENT_EMPTY,
    EVENT_DOG_NEARBY,
    EVENT_BUTTON,
    EVENT_TIMEOUT_WAIT,
    EVENT_TIMEOUT_CLOSE_SERVO,
    EVENT_DOG_AWAY,     //agrego evento para detectar cuando se aleja el perro (para que no siga tirando pelotas)
    EVENT_SEND_STATE_BT,
    EVENT_CONTINUE
};

// ------------------------------------------------
// Estructura de event
// ------------------------------------------------
typedef struct event_s
{
    event_e type;
    int value;
} event_t;


// ------------------------------------------------
// Variables globales
// ------------------------------------------------
state_e actual_state;
event_t event;

unsigned long previous_time;
unsigned long current_time;

unsigned long time_waitDog_since;
unsigned long time_waitDog_until;
bool check_time_waitDog = false;

unsigned long time_servo_since;
unsigned long time_servo_until;
bool check_time_servo = false;

bool dogDetected;
bool emptyMessage;



// ------------------------------------------------
// Logs
// ------------------------------------------------
void log(const char *state, const char *event)
{

    Serial.println("------------------------------------------------");
    Serial.println(state);
    Serial.println(event);
    Serial.println("------------------------------------------------");

}

void log(const char *msg)
{

    Serial.println(msg);

}

void log(int val)
{

    Serial.println(val);

}
// ------------------------------------------------
// FUNCTIONS INIT
// ------------------------------------------------
void servo_init();
void rgb_init();
void button_init();
void distance_dog_init();
void distance_ball_init();

// ------------------------------------------------
// Verificar sensores
// ------------------------------------------------
void verify_distance_ball();
void verify_distance_dog();
bool verify_button();  
// ------------------------------------------------
// Inicialización
// ------------------------------------------------
void start()
{
    Serial.begin(9600);
    bluetooth.begin(9600);
  	servo_init();
    rgb_init();
    button_init();
    distance_dog_init();
    distance_ball_init();
    dogDetected = false;
    emptyMessage = true;
  	actual_state = STATE_CHECKING;
    bluetooth.println("Arduino Conectado!");
    bluetooth_send_state();
    previous_time = millis();
}
// ------------------------------------------------
// Implementación maquina de estados
// ------------------------------------------------
void fsm()
{
    catch_event();
    switch (actual_state)
    {
    case STATE_CHECKING:
        switch (event.type)
        {
            case EVENT_NOT_EMPTY:
                update_led(GREEN);
                log("STATE_CHECKING", "EVENT_NOT_EMPTY");
                actual_state = STATE_READY;
                bluetooth_send_state();
                emptyMessage = true; // la proxima vez que no haya pelota, avisa con mensaje por BT
                break;

            case EVENT_EMPTY:
                update_led(RED);
                log("STATE_CHECKING", "EVENT_EMPTY");
                send_empty_message();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_BUTTON:
                log("STATE_CHECKING", "EVENT_BUTTON");
                emptyMessage = true;
                send_empty_message();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_SEND_STATE_BT:
                log("STATE_CHECKING", "EVENT_SEND_STATE_BT");
                bluetooth_send_state();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_CONTINUE:
                log("STATE_CHECKING", "EVENT_CONTINUE");
                actual_state = STATE_CHECKING;
                break;

            default:
                break;
        }
        break;

    case STATE_READY:
        switch (event.type)
        {
            case EVENT_DOG_NEARBY:
                update_led(YELLOW);
                time_waitDog_since = millis();
                check_time_waitDog = true;
                dogDetected = true;
                log("STATE_READY", "EVENT_DOG_NEARBY");
                actual_state = STATE_DOG_DETECTED;
                bluetooth_send_state();
                break;

            case EVENT_BUTTON:
                update_led(YELLOW);
                drop_ball();
                dogDetected = true;
                log("STATE_READY", "EVENT_BUTTON");
                actual_state = STATE_DROP_BALL;
                bluetooth_send_state();
                break;
            
            case EVENT_SEND_STATE_BT:
                dogDetected = true;
                log("STATE_READY", "EVENT_SEND_STATE_BT");
                bluetooth_send_state();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_CONTINUE:
                log("STATE_READY", "EVENT_CONTINUE");
                actual_state = STATE_READY;
                break;

            default:
                break;
        }
        break;

    case STATE_DOG_DETECTED:
        switch (event.type)
        {
            case EVENT_TIMEOUT_WAIT:
                drop_ball();
                log("STATE_DOG_DETECTED", "EVENT_TIME_OUT_WAIT");
                actual_state = STATE_DROP_BALL;
                bluetooth_send_state();
                break;

            case EVENT_BUTTON:
                drop_ball();
                dogDetected = true;
                log("STATE_DOG_DETECTED", "EVENT_BUTTON");
                actual_state = STATE_DROP_BALL;
                bluetooth_send_state();
                break;

            case EVENT_SEND_STATE_BT:
                dogDetected = true;
                log("STATE_DOG_DETECTED", "EVENT_SEND_STATE_BT");
                bluetooth_send_state();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_CONTINUE:
                log("STATE_DOG_DETECTED", "EVENT_CONTINUE");
                actual_state = STATE_DOG_DETECTED;
                break;

            default:
                break;
        }
        break;

    case STATE_DROP_BALL:
        switch (event.type)
        {
            case EVENT_TIMEOUT_CLOSE_SERVO:
                close_servo();
                update_led(NONE);
                log("STATE_DROP_BALL", "EVENT_TIMEOUT_CLOSE_SERVO");
                actual_state = STATE_END_OF_SERVICE;
                bluetooth_send_state();
                break;

            case EVENT_SEND_STATE_BT:
                log("STATE_DROP_BALL", "EVENT_SEND_STATE_BT");
                bluetooth_send_state();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_CONTINUE:
                log("STATE_DROP_BALL", "EVENT_CONTINUE");
                actual_state = STATE_DROP_BALL;
                break;

            default:
                break;
        }
        break;

    case STATE_END_OF_SERVICE:
        switch (event.type)
        {
            case EVENT_DOG_AWAY:

                log("STATE_END_OF_SERVICE", "EVENT_DOG_AWAY");
                actual_state = STATE_CHECKING;
                bluetooth_send_state();
                dogDetected = false;
                break;

            case EVENT_SEND_STATE_BT:
                log("STATE_END_OF_SERVICE", "EVENT_SEND_STATE_BT");
                bluetooth_send_state();
                actual_state = STATE_CHECKING;
                break;

            case EVENT_CONTINUE:
                log("STATE_END_OF_SERVICE", "EVENT_CONTINUE");
                actual_state = STATE_END_OF_SERVICE;
                break;
                
            default:
                break;
        }
        break;
    }
    event.type = EVENT_CONTINUE;
    event.value = VALUE_CONTINUE;
}



// ------------------------------------------------
// Actuadores
// ------------------------------------------------
void update_led(int color)
{
    
    switch (color)
    {
        case GREEN:
            digitalWrite(PIN_LED_RED, LOW);
            digitalWrite(PIN_LED_GREEN, HIGH);
            digitalWrite(PIN_LED_BLUE, LOW);
            break;
        case YELLOW:
            digitalWrite(PIN_LED_RED, HIGH);
            digitalWrite(PIN_LED_GREEN, HIGH);
            digitalWrite(PIN_LED_BLUE, LOW);
            break;
        case RED:
            digitalWrite(PIN_LED_RED, HIGH);
            digitalWrite(PIN_LED_GREEN, LOW);
            digitalWrite(PIN_LED_BLUE, LOW);
            break;
        case NONE:
            digitalWrite(PIN_LED_RED, LOW);
            digitalWrite(PIN_LED_GREEN, LOW);
            digitalWrite(PIN_LED_BLUE, LOW);
            break;
    }
}

void bluetooth_send_state()
{
    switch(actual_state)
    {
    case STATE_CHECKING:
        bluetooth.print("CHECKING");
        break;
    case STATE_READY:
        bluetooth.print("READY");
        break;
    case STATE_DOG_DETECTED:
        bluetooth.print("DOG_DETECTED");
        break;
    case STATE_DROP_BALL:
        bluetooth.print("DROP_BALL");
        break;
    case STATE_END_OF_SERVICE:
        bluetooth.print("END_OF_SERVICE");
        break;
    }
}

void send_empty_message()
{
    if(emptyMessage)
    {
        bluetooth.println("There are no balls.");
        emptyMessage = false;
    }
}


// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
int index = 0;
void (*checkSensor[2])() = {verify_distance_dog, verify_distance_ball};

void catch_event()
{

    //verifico evento timeout de espera para tirar pelota
    if (check_time_waitDog)
    {
        time_waitDog_until = millis();
        if ((time_waitDog_until - time_waitDog_since) > TIME_WAIT_MILIS)
        {
            event.type = EVENT_TIMEOUT_WAIT;
            time_waitDog_since = time_waitDog_until;
            check_time_waitDog = false;
            return;
        }
    }

    //verifico evento timeout de espera para cerrar puerta (servo)
    if (check_time_servo)
    {
        time_servo_until = millis();
        if ((time_servo_until - time_servo_since) > TIME_SERVO_MILIS)
        {
            event.type = EVENT_TIMEOUT_CLOSE_SERVO;
            time_servo_since = time_servo_until;
            check_time_servo = false;
            return;
        }
    }

    //verifico pulsacion del boton o pedido de la app por bluetooth
    if(verify_button() == true)
        return;
    if(verify_bluetooth())
        return;

    //verifico sensores distancia
    current_time = millis();
    if ((current_time - previous_time) > TIME_EVENT_MILIS)
    {
        checkSensor[index]();
        index = ++index % 2;
        previous_time = current_time;
    }
    else
    {
        event.type = EVENT_CONTINUE;
        event.value = VALUE_CONTINUE;
    }

}
// ------------------------------------------------
// Verificar sensores
// ------------------------------------------------
void verify_distance_dog() 
{
    int distance = distance_read(DISTANCE_SENSOR_PINTRIG_DOG, DISTANCE_SENSOR_PINECHO_DOG);
    //bluetooth.println(distance);
    //bluetooth.println(dogDetected);
    if(!dogDetected)
    {
        if(distance < UMBRAL_DISTANCE_DOG)  
        {
            event.type = EVENT_DOG_NEARBY;
            //dogDetected = true;
        }
    }
    else
    {
        if(distance > UMBRAL_DISTANCE_DOG)  
        {
            event.type = EVENT_DOG_AWAY;
            //dogDetected = false;
        }
    }
}

void verify_distance_ball() 
{
    int distance = distance_read(DISTANCE_SENSOR_PINTRIG_BALL, DISTANCE_SENSOR_PINECHO_BALL);
    //bluetooth.println(distance);
    if(distance < UMBRAL_DISTANCE_BALL)
    { 
        log(distance);
        event.type = EVENT_NOT_EMPTY;
    }
    else
    {
        log(distance);
        event.type = EVENT_EMPTY;
    }
}

bool verify_button() 
{

    int button_value = digitalRead(PIN_BUTTON);
    if(button_value == HIGH)
    {
        //dogDetected = true;
        event.type = EVENT_BUTTON;
        return true;
    }
    return false;
}


bool verify_bluetooth()
{
  if(bluetooth.available())
  {
    bt_msg = bluetooth.read();
    //bluetooth.println("Lanzamiento Manual");
    switch (bt_msg)
    {
        case BLUETOOTH_BUTTON:
            //dogDetected = true;
            event.type = EVENT_BUTTON;
            return true;
            break;

        case BLUETOOTH_STATE_REQUEST:
            event.type = EVENT_SEND_STATE_BT;
            return true;
            break;

        default:
            break;
    }
  }
  return false;
}

long distance_read(int distance_pintrig, int distance_pinecho) 
{
    digitalWrite(distance_pintrig, LOW);
    delayMicroseconds(DELAY_PULSE_2);

    digitalWrite(distance_pintrig, HIGH);
    delayMicroseconds(DELAY_PULSE_10);
  
    digitalWrite(distance_pintrig, LOW);
    pinMode(distance_pinecho, INPUT);

    return (pulseIn(distance_pinecho, HIGH))* SPEED_OF_SOUND_CM_PER_MICROSECOND; 
}
//---------------------------
// Servo Implementacion
//---------------------------

void servo_init()
{
    Servomotor.attach(SERVO_PIN);
    Servomotor.write(SERVO_CLOSE);
}

void distance_ball_init()
{
  	pinMode(DISTANCE_SENSOR_PINECHO_DOG, INPUT);
  	pinMode(DISTANCE_SENSOR_PINTRIG_DOG, OUTPUT);
}
void distance_dog_init()
{
  	pinMode(DISTANCE_SENSOR_PINECHO_BALL, INPUT);
  	pinMode(DISTANCE_SENSOR_PINTRIG_BALL, OUTPUT);
}

void drop_ball()
{
    Servomotor.write(SERVO_OPEN);
    time_servo_since = millis();
    check_time_servo = true;
}

void close_servo()
{
    Servomotor.write(SERVO_CLOSE);
}


void rgb_init()
{
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_BLUE, LOW);
}

void button_init()
{
    pinMode(PIN_BUTTON, INPUT);
}
// ------------------------------------------------
// Arduino setup
// ------------------------------------------------
void setup()
{
    start();
}

// ------------------------------------------------
// Arduino loop
// ------------------------------------------------
void loop()
{
    fsm();
}