#ifndef FORMATKML_H
#define FORMATKML_H

#include <stdio.h>

/**
 * @brief Declaraciones de funciones públicas para generar archivos KML.
 */
void KMLHeader(void);
void KMLFooter(void);
void KMLLinePoint(double lon, double lat);
void KMLPlacemark(double lon, double lat, float ruido);

/**
 * @brief Cabecera del archivo KML.
 * Contiene información XML básica y estilos para el trazado de rutas.
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
 * @brief Pie del archivo KML.
 * Cierra la sección de coordenadas y el documento KML.
 */
const char* footer = "    </coordinates>\n"
                     "   </LineString>\n"
                     "  </Placemark>\n"
                     " </Document>\n"
                     "</kml>\n";

/**
 * @brief Imprime la cabecera del archivo KML en la consola.
 */
void KMLHeader() {
    printf("%s", cabecera);
}

/**
 * @brief Imprime el pie del archivo KML en la consola.
 */
void KMLFooter() {
    printf("%s", footer);
}

/**
 * @brief Imprime un punto en la línea de ruta (coordenadas simples).
 *
 * @param lon Longitud.
 * @param lat Latitud.
 */
void KMLLinePoint(double lon, double lat) {
    printf("%.6f,%.6f\n", lon, lat);
}

/**
 * @brief Imprime un punto individual con etiqueta de dB (Placemark independiente).
 *
 * @param lon Longitud.
 * @param lat Latitud.
 * @param ruido Nivel de ruido en decibelios (dB).
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