#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>

#define DEBUG  // Definición para habilitar depuración si es necesario

// Variables globales
int contador = 0;  // Contador de permutaciones gráciles encontradas
LARGE_INTEGER comienzo, frecuencia;  // Variables para medir el tiempo de ejecución
long long tiempo_limite;  // Límite de tiempo en microsegundos
bool tiempo_agotado = false;  // Bandera para indicar si el tiempo se agotó

// Función que calcula el factorial de un número entero positivo n.
unsigned long long factorial(int n) {
    // Variable para almacenar el resultado del factorial.
    // Se usa 'unsigned long long' porque los valores factoriales pueden ser grandes.
    unsigned long long resultado = 1;

    // Bucle que multiplica desde 2 hasta n.
    // Se omite multiplicar por 1 porque no cambia el resultado.
    for (int i = 2; i <= n; i++) {
        resultado *= i; // Acumulación del producto factorial.
    }

    return resultado; // Devuelve el valor final de n!
}

// Función recursiva que implementa el algoritmo de backtracking con poda.
// Parámetros:
// - perm[]: Arreglo que almacena la permutación parcial construida.
// - usado[]: Arreglo booleano que indica qué números ya han sido utilizados en la permutación actual.
// - diferencias[]: Arreglo booleano que marca qué diferencias entre elementos consecutivos ya han aparecido.
// - n: Tamaño de la permutación a generar.
// - pos: Índice actual dentro de la permutación.
void backtrack(int perm[], bool usado[], bool diferencias[], int n, int pos) {
    
    // Variable para almacenar el tiempo actual del sistema.
    LARGE_INTEGER actual;
    QueryPerformanceCounter(&actual);

    // Cálculo del tiempo transcurrido desde que se inició la ejecución.
    long long tiempo_transcurrido = (actual.QuadPart - comienzo.QuadPart) * 1000000LL / frecuencia.QuadPart;
    
    // Verifica si el tiempo límite se ha alcanzado.
    if (tiempo_transcurrido >= tiempo_limite) {
        tiempo_agotado = true;  // Marca que el tiempo se ha agotado.
        return;  // Finaliza la ejecución de esta rama de búsqueda.
    }
    
    // Caso base: Si hemos llenado toda la permutación, contamos esta como válida.
    // Cuando pos == n, significa que hemos colocado n números en perm[], es decir, 
    // hemos generado una permutación completa.
    if (pos == n) {
        contador++;  // Incrementa el contador de permutaciones válidas.
        return;  // Finaliza esta rama de la recursión.
    }

    // Bucle que intenta colocar cada número del 1 al n en la permutación.
    for (int num = 1; num <= n; num++) {
        
        // Verifica si el número ya ha sido utilizado en la permutación actual.
        if (!usado[num]) {  

            // Si no es el primer número, verificar restricciones de diferencias.
            if (pos > 0) {  // Ya se han colocado al menos dos números en la permutación.
                int diff = abs(num - perm[pos - 1]);  // Calcula la diferencia absoluta con el número anterior.
                
                // Si la diferencia ya se ha usado o es inválida, descartar esta opción (poda).
                // La diferencia no va a ser cero, n puede ser mayor o igual a n o ya fue usada.
                if (diff < 1 || diff >= n || diferencias[diff]) {
                    continue; // Se salta esta iteración del bucle y prueba otro número.
                }
                
                // Marca la diferencia como utilizada en esta rama de la recursión.
                diferencias[diff] = true;  
            }
            
            // Asigna el número a la posición actual en la permutación.
            perm[pos] = num;
            usado[num] = true;  // Marca el número como utilizado.

            // Llamado recursivo para llenar la siguiente posición.
            backtrack(perm, usado, diferencias, n, pos + 1);
            
            // Si se agotó el tiempo en la recursión, terminar la búsqueda.
            if (tiempo_agotado) return;

            // *** RETROCESO (Backtracking) ***
            // Se deshacen los cambios y se regresa a la llamada anterior de la recursión para probar una nueva opción.
            // Deshacer cambios para explorar otra posible solución.
            usado[num] = false;  // Marca el número como disponible nuevamente.
            
            // Si se usó una diferencia, se desmarca para permitir nuevas combinaciones.
            if (pos > 0) {
                diferencias[abs(num - perm[pos - 1])] = false;
            }
        }
    }
}


// Función principal para contar permutaciones gráciles
int contar_permutaciones_graciles(int n) {
    if (n == 1) return 1;  // Caso especial para n=1

    // Asignación de memoria dinámica para estructuras de datos
    /*Se usa malloc y no arreglos estáticos por:
    -Tamaño dimámico: n no es conocido en tiempo de compilación por lo tanto
     se necesita asignar memoria en tiempo de ejecución.
    -Mayor flexibilidad: porque se puede cambiar n sin recompilar el programa.*/

    /*
    -Reserva un bloque de memoria para un arreglo de n enteros (int).
    -Se usa para almacenar la permutación actual durante la ejecución del algoritmo.
    */
    int *perm = malloc(n * sizeof(int));  // Arreglo para almacenar la permutación actual

    /*
    -Reserva memoria para n + 1 valores booleanos (true/false).
    -Se usa para marcar qué números ya fueron usados en la permutación actual.
    -Se asigna n + 1 posiciones porque los números van del 1 al n, por lo que necesitamos una posición extra en el arreglo para evitar confusión con índices.
    */
    bool *usado = malloc((n + 1) * sizeof(bool));  // Arreglo para marcar los números utilizados

    /*
    -Reserva memoria para n valores booleanos.
    -Se usa para rastrear las diferencias absolutas entre elementos consecutivos en la permutación.
    -Sirve como una restricción para validar si una permutación es válida.
    */
    bool *diferencias = malloc(n * sizeof(bool));  // Arreglo para marcar las diferencias utilizadas

    // Inicialización de los arreglos
    /*La memoria asignada dinámicamente con malloc no se inicializa automáticamente, lo que significa que 
    los arreglos podrían contener valores basura aleatorios.*/
    for (int i = 0; i <= n; i++) usado[i] = false; //Asegura que todos los números están disponibles al inicio.
    for (int i = 0; i < n; i++) diferencias[i] = false; //Indica que ninguna diferencia entre elementos ha sido usada en la permutación.

    contador = 0;  // Reiniciar el contador
    backtrack(perm, usado, diferencias, n, 0);  // Llamar a la función de backtracking

    // Liberación de memoria dinámica
    /*Se hace al final de la función para evitar ocupar memoria innecesariamente 
    después de que el algoritmo ha terminado.*/
    free(perm);
    free(usado);
    free(diferencias);

    return contador;
}

// Función principal del programa
int main(int argc, char *argv[]) {
    // Verificar el número de argumentos
    if (argc != 3) { //Asegura que el usuario proporciona el numero de argumentos correctos sino imprime <numero> <tiempo en minutos>
        printf("Uso: %s <numero> <tiempo en minutos>\n", argv[0]);
        return 1;
    }

    // Leer valores desde la línea de comandos
    int n = atoi(argv[1]);  // Convertir el primer argumento a entero
    int tiempo = atoi(argv[2]);  // Convertir el segundo argumento a entero

    // Validar el rango de entrada
    if (n < 0 || n > 50) {
        printf("Numero fuera de rango. Debe estar entre 0 y 50.\n");
        return 1;
    }

    // Configuración del contador de tiempo
    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&comienzo);
    tiempo_limite = tiempo * 60 * 1000000LL;  // Convertir minutos a microsegundos


    // Calcular el número total de permutaciones posibles (factorial de n-1)
    /*Ejemplo con n = 4 (restringiendo el primer elemento)
    Si el algoritmo fija el primer número en 1, solo generará las permutaciones de {2, 3, 4}:
    1 2 3 4
    1 2 4 3
    1 3 2 4
    1 3 4 2
    1 4 2 3
    1 4 3 2
*/
    /*Si n es mayor que 1, entonces calcula el factorial de (n-1).
    Si n es 1 o menor, el resultado es simplemente 1.*/
    unsigned long long total_permutaciones = (n > 1) ? factorial(n - 1) : 1;
    /*signed long long permite valores negativos, pero reduce la cantidad de valores positivos que puede almacenar.
    unsigned long long duplica la capacidad de valores positivos porque no usa un bit para el signo.
    */

    // Ejecutar el conteo de permutaciones gráciles
    int resultado = contar_permutaciones_graciles(n);

    // Medir el tiempo de ejecución total
    LARGE_INTEGER final;
    QueryPerformanceCounter(&final);
    long long microsec = (final.QuadPart - comienzo.QuadPart) * 1000000LL / frecuencia.QuadPart;

    // Mostrar los resultados
    if (tiempo_agotado) {
        printf("Tiempo agotado. Se encontraron %d permutaciones graciles antes de detenerse.\n", contador);
    } else {
        printf("El numero de permutaciones graciles de %d es: %d\n", n, resultado);
    }
    printf("Se generaron %llu grupos de %llu combinaciones cada uno.\n", n, total_permutaciones);
    printf("Tiempo: %lld [us]\n", microsec);

    return 0;
}
