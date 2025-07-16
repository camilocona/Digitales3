/**
 * @file app.c
 * @brief Programa principal que integra operaciones con I2C, temporizadores, UART y GPS para almacenar y recuperar datos en EEPROM.
 */

#include "eeprom.h"
#include "timer.h"
#include "gps.h"
#include "pico/multicore.h"
#include <stdlib.h>
#include "ruido.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <math.h>

#define LED_PIN 25

bool bstart=false,bstop=false,bstatus=false,bclean=false;
volatile bool bestado=false, benableW=false;
int tiempo=0;
volatile bool medir_ruido_flag = false;

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
            size_t len = strlen(data);
            if (len > 0 && data[len - 1] == '\n') data[len - 1] = '\0';
            printf("---- Sent utf8 encoded message: \"%s\\n\" ----\n", data);

            if(strncmp(data, "START", 5) == 0) {
                printf("Terminal disponible.\n");
                bstart=true;
            }
            else if (bstart && strncmp(data, "OK", 2) == 0) {
                printf("Iniciando captura de sonido por 10 segundos...\n");
                medir_ruido_flag = true;
            }
            else if(bstart && strncmp(data, "STATUS", 6) == 0) {
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
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }
    configure_i2c();
    multicore_launch_core1(core1_process);
    init_timer();
    configurePinUart();
    configIntUart();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    init_adc_microfono();

    printf("Tamaño struct Coordenadas: %d bytes\n", sizeof(struct Coordenadas));

    while (1) {
        if (uart_flag) {
            uart_flag = false;
            if (data == '$') {
                buffer_index = 0;
            }
            buffer[buffer_index++] = data;

            if (data == '\n') {
                buffer[buffer_index] = '\0';
                process_nmea_sentence(buffer);
                if(GPS.Status == 1){
                    gpio_put(LED_PIN, 1);
                    bestado=true;
                    benableW=true;
                } else {
                    bestado=false;
                    gpio_put(LED_PIN, 0);
                    shora=0; sminuto=0; ssegundo=0;
                } 
            }
        } else benableW=false;

        if(benableW && alarm_fired_1 && GPS.Status == 1) {
            if (!isnan(GPS.Lat) && !isnan(GPS.Lon) &&
                GPS.Lat >= -90.0 && GPS.Lat <= 90.0 &&
                GPS.Lon >= -180.0 && GPS.Lon <= 180.0) {

                struct Coordenadas datos = {GPS.Lat, GPS.Lon, 0.0f};  // solo coordenada
                writeEeprom(currentAddress, datos);
                printf("→ Posición guardada: %.6f, %.6f @ Addr=%d\n", GPS.Lat, GPS.Lon, currentAddress);
                currentAddress += sizeof(struct Coordenadas);
                if (currentAddress >= 2048) currentAddress = 0;
            }

            if(n!=0){
                printf("Leyendo desde la EEPROM...\n");
                nDataEeprom(n, currentAddress - sizeof(struct Coordenadas));
                n=0;
            }
            alarm_fired_1=false;
        }

        if (medir_ruido_flag) {
            medir_ruido_flag = false;
            printf("Midiendo nivel de ruido...\n");

            for (int i = 0; i < 20; i++) {
                float ruido = medir_ruido_dB_rapido();
                printf("Muestra %d: %.2f dB\n", i+1, ruido);

                if (GPS.Status == 1 &&
                    !isnan(GPS.Lat) && !isnan(GPS.Lon) &&
                    GPS.Lat >= -90.0 && GPS.Lat <= 90.0 &&
                    GPS.Lon >= -180.0 && GPS.Lon <= 180.0) {

                    struct Coordenadas datos = {GPS.Lat, GPS.Lon, ruido};
                    writeEeprom(currentAddress, datos);
                    printf("EEPROM Write: %.2f dB @ %.6f, %.6f\n", datos.dB, datos.dato1, datos.dato2);
                    printf("→ Escrito en EEPROM: %.2f dB @ lat=%.6f, lon=%.6f | Addr=%d\n", ruido, GPS.Lat, GPS.Lon, currentAddress);
                    currentAddress += sizeof(struct Coordenadas);
                    if (currentAddress >= 2048) currentAddress = 0;
                } else {
                    printf("No hay señal GPS válida. Muestra descartada.\n");
                }

                sleep_ms(500);
            }

            printf("Captura finalizada.\n");
        }
    }
    return 0;
}