#include <stdio.h>
#include <stdlib.h>


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

int agregar_superviviente(Refugio *r) {
    /* 1) ¿Está lleno* -> crecer con realloc usando temporal */
    if(r->n == r->capacidad) {
        int nueva_cap = (r->capacidad == 0) ? 4 : r->capacidad *2;
        Superviviente *tmp = realloc(r->lista,(size_t)nueva_cap * sizeof(Superviviente));

        if (tmp ==NULL) {
            printf("Error: no hay memoria. \n");
            return 0;  /* CConservva los anteriores */
        }
        r-> lista = tmp;    /* solo si realloc funcinó */
        r->capacidad = nueva_cap;
    }
    /* 2) Validar y guardar datos en r->lista[r->n] */
    /* r->n++ */
    return 1;
}


int main (void) {
    Refugio refugio;

    /*Inicialización esplicita a "Vacío" */
    refugio.lista =NULL;
    refugio.n  = 0;
    refugio.capacidad = 0;

    /*Pruebas manual: registrar 3 supervivientes */
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);

    printf("Refugio Inicializado: n=%d, capacidad=%d\n", refugio.n, refugio.capacidad);

    /* Liberar antes de salir */
    free(refugio.lista);
    refugio.lista = NULL;


    return 0;
}
