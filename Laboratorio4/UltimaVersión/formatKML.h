/**
 * @file formatKML.h
 * @brief Generación de archivos KML con etiquetas de nivel de ruido en dB.
 */

#ifndef FORMAT_KML_H
#define FORMAT_KML_H

#include <stdio.h>

const char* cabecera = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
                       " <Document>\n"
                       "  <name>Ruta con Nivel de Ruido</name>\n"
                       "  <description>Visualización de coordenadas y niveles de ruido en dB.</description>\n";

const char* footer = " </Document>\n"
                     "</kml>\n";

/**
 * @brief Imprime la cabecera del archivo KML en la consola.
 */
void KMLHeader() {
    printf("%s", cabecera);
}

/**
 * @brief Imprime una entrada Placemark con coordenadas y valor de ruido en dB.
 * 
 * @param lat Latitud
 * @param lon Longitud
 * @param ruido Nivel de ruido en dB
 */
void KMLPlacemark(double lat, double lon, float ruido) {
    printf("  <Placemark>\n");
    printf("   <name>%.2f dB</name>\n", ruido);
    printf("   <description>Ruido registrado</description>\n");
    printf("   <Point>\n");
    printf("    <coordinates>-%f,%f</coordinates>\n", lon, lat);
    printf("   </Point>\n");
    printf("  </Placemark>\n");
}

/**
 * @brief Imprime el pie del archivo KML en la consola.
 */
void KMLFooter() {
    printf("%s", footer);
}

#endif