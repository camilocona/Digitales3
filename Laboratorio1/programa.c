#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>

#define DEBUG

int contador = 0;
LARGE_INTEGER comienzo, frecuencia;
long long tiempo_limite;
bool tiempo_agotado = false;

// Función para calcular el factorial
unsigned long long factorial(int n) {
    unsigned long long resultado = 1;
    for (int i = 2; i <= n; i++) {
        resultado *= i;
    }
    return resultado;
}

// Función recursiva de backtracking con poda
void backtrack(int perm[], bool usado[], bool diferencias[], int n, int pos) {
    LARGE_INTEGER actual;
    QueryPerformanceCounter(&actual);
    long long tiempo_transcurrido = (actual.QuadPart - comienzo.QuadPart) * 1000000LL / frecuencia.QuadPart;
    
    if (tiempo_transcurrido >= tiempo_limite) {
        tiempo_agotado = true;
        return;
    }
    if (pos == n) {
        contador++;
        return;
    }

    for (int num = 1; num <= n; num++) {
        if (!usado[num]) {
            if (pos > 0) {
                int diff = abs(num - perm[pos - 1]);
                if (diff < 1 || diff >= n || diferencias[diff]) {
                    continue; // Poda: descartar permutación inválida
                }
                diferencias[diff] = true;
            }

            perm[pos] = num;
            usado[num] = true;

            backtrack(perm, usado, diferencias, n, pos + 1);
            if (tiempo_agotado) return;

            usado[num] = false;
            if (pos > 0) {
                diferencias[abs(num - perm[pos - 1])] = false;
            }
        }
    }
}

// Función principal para contar permutaciones gráciles
int contar_permutaciones_graciles(int n) {
    if (n == 1) return 1; // Caso especial para n=1

    int *perm = malloc(n * sizeof(int));
    bool *usado = malloc((n + 1) * sizeof(bool));
    bool *diferencias = malloc(n * sizeof(bool));

    for (int i = 0; i <= n; i++) usado[i] = false;
    for (int i = 0; i < n; i++) diferencias[i] = false;

    contador = 0;
    backtrack(perm, usado, diferencias, n, 0);

    free(perm);
    free(usado);
    free(diferencias);

    return contador;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <numero> <tiempo en minutos>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int tiempo = atoi(argv[2]);

    if (n < 0 || n > 50) {
        printf("Numero fuera de rango. Debe estar entre 0 y 50.\n");
        return 1;
    }

    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&comienzo);
    tiempo_limite = tiempo * 60 * 1000000LL; // Convertir minutos a microsegundos

    unsigned long long total_permutaciones = (n > 1) ? factorial(n - 1) : 1;
    int resultado = contar_permutaciones_graciles(n);

    LARGE_INTEGER final;
    QueryPerformanceCounter(&final);
    long long microsec = (final.QuadPart - comienzo.QuadPart) * 1000000LL / frecuencia.QuadPart;

    if (tiempo_agotado) {
        printf("Tiempo agotado. Se encontraron %d permutaciones graciles antes de detenerse.\n", contador);
        //return 0;
    } else {
        printf("El numero de permutaciones graciles de %d es: %d\n", n, resultado);
    }
    printf("Se generaron %llu grupos de %llu combinaciones cada uno.\n", n, total_permutaciones);
    printf("Tiempo: %lld [us]\n", microsec);

    return 0;
}
