/**
 * @file formatKML.h
 * @brief Programa para la generación de archivos KML a partir de datos GPS.
 */

/**
 * @brief Cabecera del archivo KML.
 * 
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
                          "    <altitudeMode>absoluto</altitudeMode>\n"
                          "    <coordinates>";
/**
 * @brief Pie del archivo KML.
 * 
 * Cierra la sección de coordenadas y el documento KML.
 */
const char* footer = "</coordinates>\n"
                     "   </LineString>\n"
                     "  </Placemark>\n"
                    " </Document>\n"
                    "</kml>";

/**
 * @brief Imprime la cabecera del archivo KML en la consola.
 */
void KMLHeader() {
    printf("%s", cabecera);
}

/**
 * @brief Imprime una entrada de coordenadas en el formato KML.
 * 
 * @param data1 Valor de la primera coordenada.
 * @param data2 Valor de la segunda coordenada.
 */
void KMLPlacemark(double data1, double data2) {
    printf("-%f,%f\n",data2,data1);
}

/**
 * @brief Imprime el pie del archivo KML en la consola.
 */
void KMLFooter() {
    printf("%s", footer);
}