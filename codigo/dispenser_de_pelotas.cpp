#include <Servo.h>

// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
#define LOG // Comentar esta linea para desactivar logs


// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define VALUE_CONTINUE -1

//---------------------------
// SERVO INIT
//---------------------------
#define SERVO_PIN 9
#define SERVO_OPEN 0
#define SERVO_CLOSE 180
Servo Servomotor;

//---------------------------
// DISTANCE SENSOR INIT
//---------------------------
#define DISTANCE_SENSOR_PIN_DOG 12
#define DISTANCE_SENSOR_PIN_BALL 10

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

// ------------------------------------------------
// states del embebido
// ------------------------------------------------
enum state_e
{
    STATE_CHECKING,
    STATE_READY,
    STATE_EMPTY,
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


// ------------------------------------------------
// Logs
// ------------------------------------------------
void log(const char *state, const char *event)
{
#ifdef LOG
    Serial.println("------------------------------------------------");
    Serial.println(state);
    Serial.println(event);
    Serial.println("------------------------------------------------");
#endif
}

void log(const char *msg)
{
#ifdef LOG
    Serial.println(msg);
#endif
}

void log(int val)
{
#ifdef LOG
    Serial.println(val);
#endif
}
// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void catch_event()
{

}
// ------------------------------------------------
// FUNCTIONS INIT
// ------------------------------------------------
servo_init()
distance_sensor_dog_init();
distance_sensor_ball_init();
rgb_init();
button_init();

// ------------------------------------------------
// Inicialización
// ------------------------------------------------
void start()
{
    servo_init()
    distance_sensor_dog_init();
    distance_sensor_ball_init();
    rgb_init();
    button_init();
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
            //Activar actuadores
            //LED VERDE
            log("STATE_CHECKING", "EVENT_NOT_EMPTY");
            state_actual = STATE_READY;
            break;

        case EVENT_EMPTY:
            //Activar actuadores
            //LED ROJO
            log("STATE_CHECKING", "EVENT_EMPTY");
            //state_actual = STATE_EMPTY;       //este no iría si lo hacemos sin estado EMPTY
            state_actual = STATE_CHECKING; //este iría si lo hacemos sin estado EMPTY
            break;

        case EVENT_CONTINUE:
            log("STATE_CHECKING", "EVENT_CONTINUE");
            state_actual = STATE_CHECKING;
            break;

        default:
            break;
        }
        break;

        //                      Este estado no iría en caso de manejar el evento EMPTY en el estado CHECKING
        // case STATE_EMPTY:
        //     switch (event.type)
        //     {
        //     case EVENT_NOT_EMPTY:
        //         //Activar actuadores
        //         //LED VERDE
        //         log("STATE_EMPTY", "EVENT_NOT_EMPTY");
        //         state_actual = STATE_READY;
        //         break;
        //     case EVENT_CONTINUE:
        //         log("STATE_EMPTY", "EVENT_CONTINUE");
        //         state_actual = STATE_EMPTY;
        //         break;
        //     }
        //     break;

    case STATE_READY:
        switch (event.type)
        {
        case EVENT_DOG_NEARBY:
            //Activar actuadores
            //LED AMARILLO
            log("STATE_READY", "EVENT_DOG_NEARBY");
            state_actual = STATE_DOG_DETECTED;
            break;

        case EVENT_BUTTON:
            //Activar actuadores
            //LED AMARILLO
            //SERVIR PELOTA
            log("STATE_READY", "EVENT_BUTTON");
            state_actual = STATE_DROP_BALL;
            break;

        case EVENT_CONTINUE:
            log("STATE_READY", "EVENT_CONTINUE");
            state_actual = STATE_READY;
            break;

        default:
            break;
        }
        break;

    case STATE_DOG_DETECTED:
        switch (event.type)
        {
        case EVENT_TIMEOUT_WAIT:
            //Activar actuadores
            //SERVIR PELOTA
            log("STATE_DOG_DETECTED", "EVENT_TIME_OUT_WAIT");
            state_actual = STATE_DROP_BALL;
            break;

        case EVENT_BUTTON:
            //Activar actuadores
            //SERVIR PELOTA
            log("STATE_DOG_DETECTED", "EVENT_BUTTON");
            state_actual = STATE_DROP_BALL;
            break;

        case EVENT_CONTINUE:
            log("STATE_DOG_DETECTED", "EVENT_CONTINUE");
            state_actual = STATE_DOG_DETECTED;
            break;

        default:
            break;
        }

    case STATE_DROP_BALL:
        switch (event.type)
        {
        case EVENT_TIMEOUT_CLOSE_SERVO:
            //Activar actuadores
            //CERRAR SERVO
            //APAGAR LED
            log("STATE_DROP_BALL", "EVENT_TIMEOUT_CLOSE_SERVO");
            state_actual = STATE_END_OF_SERVICE;
            break;

        case EVENT_CONTINUE:
            log("STATE_DROP_BALL", "EVENT_CONTINUE");
            state_actual = STATE_DROP_BALL;
            break;

        default:
            break;
        }

    case STATE_END_OF_SERVICE:
        switch (event.type)
        {
        case EVENT_DOG_AWAY:
            //Activar actuadores

            log("STATE_END_OF_SERVICE", "EVENT_DOG_AWAY");
            state_actual = STATE_CHECKING;
            break;

        case EVENT_CONTINUE:
            log("STATE_END_OF_SERVICE", "EVENT_CONTINUE");
            state_actual = STATE_END_OF_SERVICE;
            break;
            
        default:
            break;
        }
        
        break;
    }

    // Ya se atendió el event
    event.type = EVENT_CONTINUE;
    event.value = VALUE_CONTINUE;
}
//---------------------------
// Servo Implementacion
//---------------------------
void servo_init()
{
  Servomotor.attach(SERVO_PIN);
  Servomotor.write(SERVO_CLOSE);
}

void distance_sensor_dog_init()
{

}

void distance_sensor_ball_init()
{

}   

void rgb_init()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  // En que color se inicia?
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