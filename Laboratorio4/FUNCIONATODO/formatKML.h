/**
 * @file formatKML.h
 * @brief Declaraciones y definiciones para la generación de archivos KML.
 * 
 * Este módulo permite crear trazas geoespaciales visualizables en herramientas como Google Earth.
 * Se generan estructuras XML para representar rutas (LineString) y puntos individuales con niveles de ruido (Placemark).
 * 
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#ifndef FORMATKML_H
#define FORMATKML_H

#include <stdio.h> ///< Funciones estándar de entrada/salida

/**
 * @brief Imprime la cabecera del archivo KML en la consola.
 * Contiene metadatos XML, estilos y encabezado del documento.
 */
void KMLHeader(void);

/**
 * @brief Imprime el pie de cierre del archivo KML en la consola.
 * Cierra correctamente las etiquetas abiertas del documento.
 */
void KMLFooter(void);

/**
 * @brief Imprime un punto dentro de una línea de coordenadas (LineString).
 * 
 * @param lon Longitud (X).
 * @param lat Latitud (Y).
 */
void KMLLinePoint(double lon, double lat);

/**
 * @brief Imprime un punto individual con información de ruido en decibeles como placemark.
 * 
 * @param lon Longitud (X).
 * @param lat Latitud (Y).
 * @param ruido Nivel de ruido en dB a mostrar como nombre.
 */
void KMLPlacemark(double lon, double lat, float ruido);

/**
 * @brief Cadena XML que representa la cabecera del archivo KML.
 * 
 * Incluye metadatos, estilos y la estructura inicial del documento, lista para insertar coordenadas.
 */
const char* cabecera = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
                       " <Document>\n"
                       "  <name>Ruta UDEA</name>\n"
                       "  <description>Implementacion de Gps mas memoria eeprom para la captura y trazado de ruta.</description>\n"
                       "  <Style id=\"yellowLineGreenPoly\">\n"
                       "   <LineStyle>\n"
                       "    <color>7f00ffff</color>\n"
                       "    <width>4</width>\n"
                       "   </LineStyle>\n"
                       "   <PolyStyle>\n"
                       "    <color>7f00ff00</color>\n"
                       "   </PolyStyle>\n"
                       "  </Style>\n"
                       "  <Placemark>\n"
                       "   <name>Relieve absoluto</name>\n"
                       "   <description>Pared verde transparente con contornos amarillos</description>\n"
                       "   <styleUrl>#yellowLineGreenPoly</styleUrl>\n"
                       "   <LineString>\n"
                       "    <extrude>1</extrude>\n"
                       "    <tessellate>1</tessellate>\n"
                       "    <altitudeMode>clampToGround</altitudeMode>\n"
                       "    <coordinates>\n";

/**
 * @brief Cadena XML que representa el pie del archivo KML.
 * 
 * Cierra correctamente las etiquetas abiertas (`coordinates`, `LineString`, `Placemark`, `Document`, `kml`).
 */
const char* footer = "    </coordinates>\n"
                     "   </LineString>\n"
                     "  </Placemark>\n"
                     " </Document>\n"
                     "</kml>\n";

/**
 * @brief Implementación de la función KMLHeader.
 */
void KMLHeader() {
    printf("%s", cabecera);
}

/**
 * @brief Implementación de la función KMLFooter.
 */
void KMLFooter() {
    printf("%s", footer);
}

/**
 * @brief Imprime una coordenada como parte de una línea de ruta (sin etiqueta).
 * 
 * @param lon Longitud.
 * @param lat Latitud.
 */
void KMLLinePoint(double lon, double lat) {
    printf("%.6f,%.6f\n", lon, lat);
}

/**
 * @brief Imprime un marcador individual en el mapa con un valor de ruido asociado.
 * 
 * @param lon Longitud.
 * @param lat Latitud.
 * @param ruido Valor numérico en decibelios.
 */
void KMLPlacemark(double lon, double lat, float ruido) {
    printf("  <Placemark>\n");
    printf("    <name>%.2f dB</name>\n", ruido);
    printf("    <Point>\n");
    printf("      <coordinates>%.6f,%.6f</coordinates>\n", lon, lat);
    printf("    </Point>\n");
    printf("  </Placemark>\n");
}

#endif  // FORMATKML_H
