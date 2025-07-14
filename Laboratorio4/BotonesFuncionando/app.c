/**
 * @file app.c
 * @brief Programa principal que integra operaciones con I2C, temporizadores, UART y GPS para almacenar y recuperar datos en EEPROM.
 */

#include "eeprom.h"
#include "timer.h"
#include "gps.h"
#include "ruido.h"
#include "pico/multicore.h"
#include <stdlib.h>

#define LED_PIN 25
#define PULSADOR_PIN 15
#define LED_VERDE 2
#define LED_AMARILLO 3
#define LED_NARANJA 4
#define LED_ROJO 5

volatile bool midiendo = false;

bool bstart=false,bstop=false,bstatus=false,bclean=false;
volatile bool bestado=false, benableW=false;
int tiempo=0;

uint16_t writeAddr = 0, posAddr = 0;     
uint16_t currentAddress = 0;

int n=0;

char buffer[BUFFSIZE];
volatile int buffer_index = 0;

/**
 * @brief Funcion que se ejecutara en el segundo core la cual nos permite los comandos desde la terminal.
 */
void core1_process(){
    char data[11];
    while (1)
    
    {
        if (fgets(data, sizeof(data), stdin) != NULL) {
            ///< Eliminar el carácter de nueva línea del final
            size_t len = strlen(data);
            if (len > 0 && data[len - 1] == '\n') data[len - 1] = '\0';

            if(strncmp(data, "START", 5) == 0) {
                printf("Terminal disponible.\n");
                bstart=true;
            }else if(bstart && strncmp(data, "STATUS", 6) == 0) {
                printf("Estado del GPS: %s\n", (bestado) ? "Posicionado" : "No posicionado");
                if(currentAddress !=0) {
                    printf("Num datos: %d\n",((currentAddress - sizeof(struct Coordenadas)) / sizeof(struct Coordenadas)));
                    printf("Num registro: %d\n",(currentAddress - sizeof(struct Coordenadas)));
                }
                else {
                    printf("Num datos: %d\n",0);
                    printf("Num registros: %d\n",0);
                }
                if(bestado) printf("Tiempo GPS activo: %02d:%02d:%02d\n", elapsed_hours, elapsed_minutes, elapsed_seconds);
                else printf("Tiempo GPS activo: %02d:%02d:%02d\n", 0, 0, 0);
                bstatus=true;
                bclean=false;
            }else if (bstart && strncmp(data, "FETCH ", 6) == 0) {
                n = atoi(data + 6);
                bstatus=false;
                bclean=false;
            }else if(bstart && strncmp(data, "CLEAN", 5) == 0){
                currentAddress = 0;
                printf("Clean Eeprom.\n");
                printf("Posicion: %d\n",currentAddress);
                bclean=true;
                bstatus=false;
            }else if(bstart && strncmp(data, "STOP", 4) == 0){
                printf("Terminal cerrada.\n");
                bstart=false;
                bstatus=false;
                bclean=false;
            }else printf("Comando no reconocido.\n");
        }
    }
}

int main() {
    stdio_init_all();
    configure_i2c();
    multicore_launch_core1(core1_process);
    init_timer();
    configurePinUart();
    configIntUart();

    // Inicialización de LEDs y pulsador
    gpio_init(PULSADOR_PIN);
    gpio_set_dir(PULSADOR_PIN, GPIO_IN);
    gpio_pull_up(PULSADOR_PIN);

    gpio_init(LED_VERDE);
    gpio_init(LED_AMARILLO);
    gpio_init(LED_NARANJA);
    gpio_init(LED_ROJO);

    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_AMARILLO, GPIO_OUT);
    gpio_set_dir(LED_NARANJA, GPIO_OUT);
    gpio_set_dir(LED_ROJO, GPIO_OUT);

    init_adc_microfono(); // ADC para ruido

    gpio_put(LED_VERDE, 1); // Encender LED verde al inicio (espera)

    while (1) {
        if (uart_flag) {
            uart_flag = false;
            ///< Procesa datos NMEA
            if (data == '$') {
                ///< Comenzó una nueva sentencia NMEA
                buffer_index = 0;
            }
            ///< Almacena el carácter en el búfer
            buffer[buffer_index++] = data;

            ///< Verifica si se alcanzó el final de la sentencia NMEA
            if (data == '\n') {
                ///< Terminó la sentencia NMEA, procesar los datos
                buffer[buffer_index] = '\0';  ///< Agrega el terminador nulo al final del búfer
                process_nmea_sentence(buffer);
                if(GPS.Status == 1){
                    gpio_put(LED_PIN, 1);
                    bestado=true;
                    benableW=true;
                }
                else{
                    bestado=false;
                    gpio_put(LED_PIN, 0);
                    shora=0; sminuto=0; ssegundo=0;
                } 
            }
        } else benableW=false;

            if(benableW && alarm_fired_1)
        {
            float ruido = 0.0f; // Valor por defecto mientras no se mida con botón
            struct Coordenadas datos = {GPS.Lat, GPS.Lon, ruido};

            writeEeprom(currentAddress, datos);
            currentAddress += sizeof(struct Coordenadas);
            if (currentAddress >= 2048) currentAddress = 0;

            sleep_ms(1000);

            if(n!=0){
                printf("Leyendo desde la EEPROM...\n");
                nDataEeprom(n, currentAddress - sizeof(struct Coordenadas));
                n=0;
            }
            alarm_fired_1=false;
        }

        // Medición manual con pulsador
        if (gpio_get(PULSADOR_PIN) == 0) { // Pulsado (activo bajo)
            sleep_ms(50); // Antirebote
            if (gpio_get(PULSADOR_PIN) == 0) {

                if (midiendo) {
                    // Cancelar medición en curso
                    gpio_put(LED_ROJO, 1);
                    sleep_ms(3000);
                    gpio_put(LED_ROJO, 0);
                    midiendo = false;
                } else if (bestado) {
                    // Iniciar nueva medición
                    midiendo = true;
                    gpio_put(LED_VERDE, 0);
                    gpio_put(LED_AMARILLO, 1);

                    // float ruido = medir_ruido_dB(); // mide 10s internamente
                    float ruido = 50.0f; // 🔁 Valor simulado, solo para prueba


                    if (!bestado) {
                        // Se perdió el GPS en medio de la medición
                        gpio_put(LED_AMARILLO, 0);
                        gpio_put(LED_ROJO, 1);
                        sleep_ms(3000);
                        gpio_put(LED_ROJO, 0);
                        gpio_put(LED_VERDE, 1);
                        midiendo = false;
                        continue;
                    }

                    struct Coordenadas datos = { GPS.Lat, GPS.Lon, ruido };
                    writeEeprom(currentAddress, datos);
                    currentAddress += sizeof(struct Coordenadas);
                    if (currentAddress >= 2048) currentAddress = 0;

                    gpio_put(LED_AMARILLO, 0);
                    gpio_put(LED_NARANJA, 1);
                    sleep_ms(300);
                    gpio_put(LED_NARANJA, 0);
                    gpio_put(LED_VERDE, 1);
                    midiendo = false;
                } else {
                    // GPS no posicionado
                    gpio_put(LED_ROJO, 1);
                    sleep_ms(1000);
                    gpio_put(LED_ROJO, 0);
                }

                while (gpio_get(PULSADOR_PIN) == 0) tight_loop_contents(); // Esperar liberación
            }
        }
    }

    return 0;
}
