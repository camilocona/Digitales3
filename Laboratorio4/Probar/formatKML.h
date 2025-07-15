/**
 * @file formatKML.h
 * @brief Programa para la generación de archivos KML a partir de datos GPS con nivel de ruido.
 */

const char* cabecera = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
                        " <Document>\n"
                        "  <name>Ruta UDEA</name>\n"
                        "  <description>Implementación de GPS y medición de ruido ambiental</description>\n"
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
                        "   <name>Ruta</name>\n"
                        "   <styleUrl>#yellowLineGreenPoly</styleUrl>\n"
                        "   <LineString>\n"
                        "    <extrude>1</extrude>\n"
                        "    <tessellate>1</tessellate>\n"
                        "    <altitudeMode>clampToGround</altitudeMode>\n"
                        "    <coordinates>\n";

const char* footer = "    </coordinates>\n"
                     "   </LineString>\n"
                     "  </Placemark>\n"
                     " </Document>\n"
                     "</kml>";

/**
 * @brief Imprime la cabecera del archivo KML.
 */
void KMLHeader() {
    printf("%s", cabecera);
}

/**
 * @brief Imprime una entrada de coordenadas para el LineString.
 * 
 * @param lat Latitud
 * @param lon Longitud
 */
void KMLLinePoint(double lat, double lon) {
    printf("%.6f,%.6f\n", lon, lat);
}

/**
 * @brief Imprime un Placemark individual con valor de dB.
 * 
 * @param lat Latitud
 * @param lon Longitud
 * @param ruido Nivel de ruido en dB
 */
void KMLPlacemark(double lat, double lon, float ruido) {
    printf("  <Placemark>\n");
    printf("    <name>%.2f dB</name>\n", ruido);
    printf("    <Point>\n");
    printf("      <coordinates>%.6f,%.6f</coordinates>\n", lon, lat);
    printf("    </Point>\n");
    printf("  </Placemark>\n");
}

/**
 * @brief Imprime el pie del archivo KML.
 */
void KMLFooter() {
    printf("%s", footer);
}