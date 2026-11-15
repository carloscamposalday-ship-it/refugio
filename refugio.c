#include <stdio.h>

#define MAX_NOMBRE 30
#define MAX_HABILIDAD 30
#define MAX_DESC 60

typedef struct {
    char nombre [MAX_NOMBRE + 1];
    int edad;
    char habilidad[MAX_HABILIDAD + 1];
    int salud;     // 0 a 100
}Superviviente;

typedef struct {
    Superviviente *lista;
    int n;
    int capacidad; 
}Refugio;


int main (void) {
    return 0;
}
