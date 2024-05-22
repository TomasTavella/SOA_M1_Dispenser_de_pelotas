#include <Servo.h>

// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define SPEED_OF_SOUND 0.01723
#define MIN_SENSOR_FUERZA 0
#define MAX_SENSOR_FUERZA 914
#define MIN_ESCALA_NIVEL 0
#define MAX_ESCALA_NIVEL 255
#define PWM_BRILLO_MAXIMO 255
#define MIN_ANCHO_PULSO 500
#define MAX_ANCHO_PULSO 2500
#define VALOR_CONTINUE -1

// ------------------------------------------------
// TEMPORIZADORES
// ------------------------------------------------
#define TMP_EVENTOS_MILI 50
#define TMP_SERVICIO_MILI 1000

// ------------------------------------------------
// Pines sensores (A = analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_SENSOR_DISTANCIA 13
#define PIN_A_SENSOR_FUERZA A0

// ------------------------------------------------
// Pines actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_ACTUADOR_BUZZER 6
#define PIN_P_ACTUADOR_LED_DISTANCIA 5
#define PIN_D_ACTUADOR_LED_CARGA_ROJO 12
#define PIN_D_ACTUADOR_LED_CARGA_AZUL 11
#define PIN_D_ACTUADOR_LED_CARGA_VERDE 10
#define PIN_D_ACTUADOR_SERVO 9

// ------------------------------------------------
// Umbrales Distancia
// ------------------------------------------------
#define UMBRAL_LEJOS 250
#define UMBRAL_NO_TAN_CERCA 200
#define UMBRAL_CERCA 150
#define UMBRAL_MUY_CERCA 100

// ------------------------------------------------
// Umbrales Fuerza
// ------------------------------------------------
#define UMBRAL_MEDIO 238
#define UMBRAL_VACIO 0

// ------------------------------------------------
// Brillo led
// ------------------------------------------------
#define BRILLO_0_POR_CIENTO 0
#define BRILLO_10_POR_CIENTO 0.10
#define BRILLO_25_POR_CIENTO 0.25
#define BRILLO_50_POR_CIENTO 0.50
#define BRILLO_100_POR_CIENTO 1

// ------------------------------------------------
// Color led
// ------------------------------------------------
#define COLOR_LED_ROJO 0
#define COLOR_LED_AMARILLO 1
#define COLOR_LED_VERDE 2

// ------------------------------------------------
// Frecuencias Buzzer
// ------------------------------------------------
#define FREC_APAGADO 0
#define FREC_ENCENDIDO 494

// ------------------------------------------------
// Posiciones Servo
// ------------------------------------------------
#define POS_INICIAL 0
#define POS_ABIERTO 90
#define POS_CERRADO -90

// ------------------------------------------------
// Estados del embebido
// ------------------------------------------------
enum estado_e
{
    ESTADO_EMBEBIDO_NO_PREPARADO,
    ESTADO_EMBEBIDO_PREPARADO,
    ESTADO_EMBEBIDO_SIRVIENDO,
    ESTADO_EMBEBIDO_FINALIZANDO
};

// ------------------------------------------------
// Eventos posibles
// ------------------------------------------------
enum evento_e
{
    EVENTO_LLENO,
    EVENTO_MEDIO,
    EVENTO_VACIO,
    EVENTO_LIBRE,
    EVENTO_UMBRAL_MUY_CERCA,
    EVENTO_UMBRAL_CERCA,
    EVENTO_UMBRAL_NO_TAN_CERCA,
    EVENTO_UMBRAL_LEJOS,
    EVENTO_TIMEOUT_SERVICIO,
    EVENTO_CONTINUE
};

// ------------------------------------------------
// Estados del sensor distancia
// ------------------------------------------------
enum estado_sensor_distancia_e
{
    ESTADO_SENSOR_LIBRE,
    ESTADO_SENSOR_OCUPADO,
};

// ------------------------------------------------
// Estructura de evento
// ------------------------------------------------
typedef struct evento_s
{
    evento_e tipo;
    int valor;
} evento_t;

// ------------------------------------------------
// Estructura de sensor
// ------------------------------------------------
typedef struct sensor_distancia_s
{
    int pin;
    int estado;
    float cm;
} sensor_distancia_t;

typedef struct sensor_fuerza_s
{
    int pin;
    int nivel;
} sensor_fuerza_t;

typedef struct actuador_led_s
{
    int pin;
    int brillo;
} actuador_led_t;

typedef struct actuador_led_rgb_s
{
    int pin_rojo;
    int pin_azul;
    int pin_verde;
    int color;
} actuador_led_rgb_t;

typedef struct actuador_buzzer_s
{
    int pin;
    int frecuencia;
} actuador_buzzer_t;

// ------------------------------------------------
// Variables globales
// ------------------------------------------------
estado_e estado_actual;
evento_t evento;
sensor_distancia_t sensor_distancia;
sensor_fuerza_t sensor_fuerza;
actuador_led_t actuador_led;
actuador_led_rgb_t actuador_led_rgb;
actuador_buzzer_t actuador_buzzer;
Servo actuador_servo;

unsigned long timepo_anterior;
unsigned long timepo_actual;

unsigned long tiempo_servicio_desde;
unsigned long tiempo_servicio_hasta;
bool verificar_tiempo_servicio;

// ------------------------------------------------
// Logs
// ------------------------------------------------
void log(const char *estado, const char *evento)
{
#ifdef LOG
    Serial.println("------------------------------------------------");
    Serial.println(estado);
    Serial.println(evento);
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
// Logica de sensores
// ------------------------------------------------
long leer_sensor_distancia(int pin)
{
    long tiempo_pulso;

    // Configuro el pin como salida en estado bajo
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(2);

    // Emito la señal de medición por 5 microsegundos
    digitalWrite(pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(pin, LOW);

    // Configuro el pin como entrada y leo el tiempo de eco
    pinMode(pin, INPUT);
    tiempo_pulso = pulseIn(pin, HIGH);

    // Convierto a centimetros
    return tiempo_pulso * SPEED_OF_SOUND;
}

int leer_sensor_fuerza(int pin)
{
    int lectura = analogRead(pin);

    /*  El sensor de fuerza varia su resistencia entregando un  valor
        de 0-914 representando un rango de fuerzas de 0-10 Newton
        Lo mapeamos a valores discretos de 0 a 255 representando el nivel de carga

        0 = 0N = 0kg = 0% (Vacio)
        238 ~= 5N = ~= 0.5kg = 50% (Medio)
        255 ~= 10N ~= 1.0kg = 100% (lleno)
    */
    return map(lectura, MIN_SENSOR_FUERZA, MAX_SENSOR_FUERZA, MIN_ESCALA_NIVEL, MAX_ESCALA_NIVEL);
}

void varificarEstadoSensorDistancia()
{
    sensor_distancia.cm = leer_sensor_distancia(sensor_distancia.pin);

    switch (sensor_distancia.estado)
    {
    case ESTADO_SENSOR_LIBRE:
        if (sensor_distancia.cm <= UMBRAL_LEJOS)
        {
            evento.tipo = EVENTO_UMBRAL_LEJOS;
            sensor_distancia.estado = ESTADO_SENSOR_OCUPADO;
        }
        break;

    case ESTADO_SENSOR_OCUPADO:
        if (sensor_distancia.cm >= UMBRAL_LEJOS)
        {
            evento.tipo = EVENTO_LIBRE;
            sensor_distancia.estado = ESTADO_SENSOR_LIBRE;
        }
        else if (sensor_distancia.cm > UMBRAL_NO_TAN_CERCA && sensor_distancia.cm <= UMBRAL_LEJOS)
        {
            evento.tipo = EVENTO_UMBRAL_LEJOS;
            sensor_distancia.estado = ESTADO_SENSOR_OCUPADO;
        }
        else if (sensor_distancia.cm > UMBRAL_CERCA && sensor_distancia.cm <= UMBRAL_NO_TAN_CERCA)
        {
            evento.tipo = EVENTO_UMBRAL_NO_TAN_CERCA;
            sensor_distancia.estado = ESTADO_SENSOR_OCUPADO;
        }
        else if (sensor_distancia.cm >= UMBRAL_MUY_CERCA && sensor_distancia.cm < UMBRAL_CERCA)
        {
            evento.tipo = EVENTO_UMBRAL_CERCA;
            sensor_distancia.estado = ESTADO_SENSOR_OCUPADO;
        }
        else if (sensor_distancia.cm < UMBRAL_MUY_CERCA)
        {
            evento.tipo = EVENTO_UMBRAL_MUY_CERCA;
            sensor_distancia.estado = ESTADO_SENSOR_OCUPADO;
        }
    default:
        break;
    }
    log("Verificando sensor distancia");
    log(sensor_distancia.cm);
}

void verificarEstadoSensorFuerza()
{
    sensor_fuerza.nivel = leer_sensor_fuerza(sensor_fuerza.pin);

    if (sensor_fuerza.nivel > UMBRAL_MEDIO)
    {
        evento.tipo = EVENTO_LLENO;
    }
    else if (sensor_fuerza.nivel > UMBRAL_VACIO && sensor_fuerza.nivel <= UMBRAL_MEDIO)
    {
        evento.tipo = EVENTO_MEDIO;
    }
    else if (sensor_fuerza.nivel == UMBRAL_VACIO)
    {
        evento.tipo = EVENTO_VACIO;
    }
    log("Verificando sensor fuerza");
    log(sensor_fuerza.nivel);
}

// ------------------------------------------------
// Logica de actuadores
// ------------------------------------------------
void actualizar_led_carga(int color)
{
    actuador_led_rgb.color = color;
    switch (actuador_led_rgb.color)
    {
    case COLOR_LED_VERDE:
        digitalWrite(actuador_led_rgb.pin_rojo, LOW);
        digitalWrite(actuador_led_rgb.pin_verde, HIGH);
        digitalWrite(actuador_led_rgb.pin_azul, LOW);
        break;
    case COLOR_LED_AMARILLO:
        digitalWrite(actuador_led_rgb.pin_rojo, HIGH);
        digitalWrite(actuador_led_rgb.pin_verde, HIGH);
        digitalWrite(actuador_led_rgb.pin_azul, LOW);
        break;
    case COLOR_LED_ROJO:
        digitalWrite(actuador_led_rgb.pin_rojo, HIGH);
        digitalWrite(actuador_led_rgb.pin_verde, LOW);
        digitalWrite(actuador_led_rgb.pin_azul, LOW);
        break;
    }
}

void actualizar_indicador_distancia(float porcentaje)
{
    actuador_led.brillo = round(PWM_BRILLO_MAXIMO * porcentaje);
    analogWrite(actuador_led.pin, actuador_led.brillo);
}

void reproducir_sonido()
{
    actuador_buzzer.frecuencia = FREC_ENCENDIDO;
    analogWrite(actuador_buzzer.pin, actuador_buzzer.frecuencia);
}

void detener_sonido()
{
    actuador_buzzer.frecuencia = FREC_APAGADO;
    analogWrite(actuador_buzzer.pin, actuador_buzzer.frecuencia);
}

void mover_servo(int posicion)
{
    actuador_servo.write(posicion);
}

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
int indice = 0;
void (*verificar_sensor[2])() = {verificarEstadoSensorFuerza, varificarEstadoSensorDistancia};

void tomar_evento()
{

    // Verificar temporizador de secuencia de servicio
    if (verificar_tiempo_servicio)
    {
        tiempo_servicio_hasta = millis();
        if ((tiempo_servicio_hasta - tiempo_servicio_desde) > TMP_SERVICIO_MILI)
        {
            evento.tipo = EVENTO_TIMEOUT_SERVICIO;
            tiempo_servicio_desde = tiempo_servicio_hasta;
            return;
        }
    }

    // verificar sensores
    timepo_actual = millis();
    if ((timepo_actual - timepo_anterior) > TMP_EVENTOS_MILI)
    {
        verificar_sensor[indice]();
        indice = ++indice % 2;
        timepo_anterior = timepo_actual;
    }
    else
    {
        evento.tipo = EVENTO_CONTINUE;
        evento.valor = VALOR_CONTINUE;
    }
}

// ------------------------------------------------
// Inicialización
// ------------------------------------------------
void start()
{
    Serial.begin(9600);

    // Asigno los pines a los sensores correspondientes
    sensor_fuerza.pin = PIN_A_SENSOR_FUERZA;
    pinMode(sensor_fuerza.pin, INPUT);

    sensor_distancia.pin = PIN_D_SENSOR_DISTANCIA;
    sensor_distancia.estado = ESTADO_SENSOR_LIBRE;

    // Asigno los pines al actuador led indicador de distancia
    actuador_led.pin = PIN_P_ACTUADOR_LED_DISTANCIA;
    pinMode(actuador_led.pin, OUTPUT);

    // Asigno los pines al actuador led indicador de carga
    actuador_led_rgb.pin_rojo = PIN_D_ACTUADOR_LED_CARGA_ROJO;
    actuador_led_rgb.pin_verde = PIN_D_ACTUADOR_LED_CARGA_VERDE;
    actuador_led_rgb.pin_azul = PIN_D_ACTUADOR_LED_CARGA_AZUL;
    pinMode(actuador_led_rgb.pin_rojo, OUTPUT);
    pinMode(actuador_led_rgb.pin_verde, OUTPUT);
    pinMode(actuador_led_rgb.pin_azul, OUTPUT);

    // Asigno los pines al actuador buzzer
    actuador_buzzer.pin = PIN_D_ACTUADOR_BUZZER;
    pinMode(actuador_buzzer.pin, OUTPUT);

    // Asigno los pines al actuador servo
    actuador_servo.attach(PIN_D_ACTUADOR_SERVO, MIN_ANCHO_PULSO, MAX_ANCHO_PULSO);
    actuador_servo.write(POS_INICIAL);

    // Inicializo el estado del embebido
    estado_actual = ESTADO_EMBEBIDO_NO_PREPARADO;

    // Inicializo el temporizador
    timepo_anterior = millis();
}

// ------------------------------------------------
// Implementación maquina de estados
// ------------------------------------------------
void fsm()
{
    // sensar peso y distancia
    tomar_evento();

    switch (estado_actual)
    {
    case ESTADO_EMBEBIDO_NO_PREPARADO:
        switch (evento.tipo)
        {
        case EVENTO_LLENO:
            actualizar_led_carga(COLOR_LED_VERDE);
            log("ESTADO_EMBEBIDO_NO_PREPARADO", "EVENTO_LLENO");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_MEDIO:
            actualizar_led_carga(COLOR_LED_AMARILLO);
            log("ESTADO_EMBEBIDO_NO_PREPARADO", "EVENTO_MEDIO");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_VACIO:
            actualizar_led_carga(COLOR_LED_ROJO);
            actualizar_indicador_distancia(BRILLO_0_POR_CIENTO);
            log("ESTADO_EMBEBIDO_NO_PREPARADO", "EVENTO_VACIO");
            estado_actual = ESTADO_EMBEBIDO_NO_PREPARADO;
            break;
        case EVENTO_CONTINUE:
            log("ESTADO_EMBEBIDO_NO_PREPARADO", "EVENTO_CONTINUE");
            estado_actual = ESTADO_EMBEBIDO_NO_PREPARADO;
            break;
        default:
            break;
        }
        break;

    case ESTADO_EMBEBIDO_PREPARADO:
        switch (evento.tipo)
        {
        case EVENTO_LLENO:
            actualizar_led_carga(COLOR_LED_VERDE);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_LLENO");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_MEDIO:
            actualizar_led_carga(COLOR_LED_AMARILLO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_MEDIO");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_VACIO:
            actualizar_led_carga(COLOR_LED_ROJO);
            actualizar_indicador_distancia(BRILLO_0_POR_CIENTO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_VACIO");
            estado_actual = ESTADO_EMBEBIDO_NO_PREPARADO;
            break;
        case EVENTO_LIBRE:
            actualizar_indicador_distancia(BRILLO_0_POR_CIENTO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_LIBRE");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_UMBRAL_LEJOS:
            actualizar_indicador_distancia(BRILLO_10_POR_CIENTO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_UMBRAL_LEJOS");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_UMBRAL_NO_TAN_CERCA:
            actualizar_indicador_distancia(BRILLO_25_POR_CIENTO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_UMBRAL_NO_TAN_CERCA");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_UMBRAL_CERCA:
            actualizar_indicador_distancia(BRILLO_50_POR_CIENTO);
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_UMBRAL_CERCA");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        case EVENTO_UMBRAL_MUY_CERCA:
            actualizar_indicador_distancia(BRILLO_100_POR_CIENTO);
            reproducir_sonido();
            tiempo_servicio_desde = millis();
            verificar_tiempo_servicio = true;
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_UMBRAL_MUY_CERCA");
            estado_actual = ESTADO_EMBEBIDO_SIRVIENDO;
            break;
        case EVENTO_CONTINUE:
            log("ESTADO_EMBEBIDO_PREPARADO", "EVENTO_CONTINUE");
            estado_actual = ESTADO_EMBEBIDO_PREPARADO;
            break;
        default:
            break;
        }
        break;

    case ESTADO_EMBEBIDO_SIRVIENDO:
        switch (evento.tipo)
        {
        case EVENTO_TIMEOUT_SERVICIO:
            detener_sonido();
            mover_servo(POS_ABIERTO);
            tiempo_servicio_desde = millis();
            verificar_tiempo_servicio = true;
            log("ESTADO_EMBEBIDO_SIRVIENDO", "EVENTO_TIMEOUT_SERVICIO");
            estado_actual = ESTADO_EMBEBIDO_FINALIZANDO;
            break;
        case EVENTO_CONTINUE:
            log("ESTADO_EMBEBIDO_SIRVIENDO", "EVENTO_CONTINUE");
            estado_actual = ESTADO_EMBEBIDO_SIRVIENDO;
            break;
        default:
            break;
        }
        break;

    case ESTADO_EMBEBIDO_FINALIZANDO:
        switch (evento.tipo)
        {
        case EVENTO_LIBRE:
            actualizar_indicador_distancia(BRILLO_0_POR_CIENTO);
            log("ESTADO_EMBEBIDO_FINALIZANDO", "EVENTO_LIBRE");
            estado_actual = ESTADO_EMBEBIDO_NO_PREPARADO;
            break;
        case EVENTO_TIMEOUT_SERVICIO:
            mover_servo(POS_CERRADO);
            verificar_tiempo_servicio = false;
            log("ESTADO_EMBEBIDO_FINALIZANDO", "EVENTO_TIMEOUT_SERVICIO");
            estado_actual = ESTADO_EMBEBIDO_FINALIZANDO;
            break;
        case EVENTO_CONTINUE:
            log("ESTADO_EMBEBIDO_FINALIZANDO", "EVENTO_CONTINUE");
            estado_actual = ESTADO_EMBEBIDO_FINALIZANDO;
            break;
        default:
            break;
        }
        break;
    }

    // Ya se atendió el evento
    evento.tipo = EVENTO_CONTINUE;
    evento.valor = VALOR_CONTINUE;
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
