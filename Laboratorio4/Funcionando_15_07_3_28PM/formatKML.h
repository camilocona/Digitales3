/**
 * @file formatKML.h
 * @brief Generación de archivos KML con ruta y puntos de ruido separados.
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
                     "  </Placemark>\n";

void KMLHeader() {
    printf("%s", cabecera);
}

void KMLLinePoint(double lon, double lat) {
    printf("%.6f,%.6f\n", lon, lat);
}

void KMLPlacemark(double lon, double lat, float ruido) {
    printf("  <Placemark>\n");
    printf("    <name>%.2f dB</name>\n", ruido);
    printf("    <Point>\n");
    printf("      <coordinates>%.6f,%.6f</coordinates>\n", lon, lat);
    printf("    </Point>\n");
    printf("  </Placemark>\n");
}


void KMLFooter() {
    printf("%s", footer);
    printf(" </Document>\n</kml>\n");
}