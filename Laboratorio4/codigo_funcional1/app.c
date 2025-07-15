/**
 * @file app.c
 * @brief Programa principal que integra operaciones con I2C, temporizadores, UART y GPS para almacenar y recuperar datos en EEPROM.
 */

#include "eeprom.h"
#include "timer.h"
#include "gps.h"
#include "ruido.h"  // ✅ Micrófono
#include "pico/multicore.h"
#include <stdlib.h>

#define LED_PIN 25

bool bstart=false, bstop=false, bstatus=false, bclean=false;
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
    init_adc_microfono();  // ✅ Inicialización del micrófono

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        if (uart_flag) {
            uart_flag = false;
            if (data == '$') buffer_index = 0;
            buffer[buffer_index++] = data;

            if (data == '\n') {
                buffer[buffer_index] = '\0';
                process_nmea_sentence(buffer);
                if(GPS.Status == 1){
                    gpio_put(LED_PIN, 1);
                    bestado = true;
                    benableW = true;
                } else {
                    bestado = false;
                    gpio_put(LED_PIN, 0);
                    shora = sminuto = ssegundo = 0;
                }
            }
        } else benableW = false;

        if (benableW && alarm_fired_1)
        {
            struct Coordenadas datos = {GPS.Lat, GPS.Lon};
            writeEeprom(currentAddress, datos);
            currentAddress += sizeof(struct Coordenadas);
            if (currentAddress >= 2048) currentAddress = 0;

            if(n != 0){
                printf("Leyendo desde la EEPROM...\n");
                nDataEeprom(n, currentAddress - sizeof(struct Coordenadas));
                n = 0;
            }

            alarm_fired_1 = false;
        }

        // ✅ Medición de ruido sin afectar el flujo del programa
        if (alarm_fired_2) {
            float db = medir_ruido_dB();
            printf("Nivel de ruido: %.2f dB\n", db);
            alarm_fired_2 = false;
        }
    }

    return 0;
}
