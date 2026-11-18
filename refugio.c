#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    int agua;
    int comida;
    int medicamentos;
    int municiones;
} Recursos;

typedef struct {
    Superviviente *lista;
    int n;
    int capacidad; 
    Recursos recursos; 
}Refugio;

static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}


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
    
        /* 2) Pedir y validar datos */
    Superviviente s;

    printf("Nombre: ");
    if (fgets(s.nombre, sizeof(s.nombre), stdin) == NULL) return 0;
    s.nombre[strcspn(s.nombre, "\n")] = '\0';
    if (s.nombre[0] == '\0') {
        printf("Nombre no puede estar vacio.\n");
        return 0;
    }

    printf("Edad: ");
    if (scanf("%d", &s.edad) != 1) { limpiar_buffer(); return 0; }
    limpiar_buffer();
    if (s.edad <= 0) {
        printf("La edad debe ser mayor que 0.\n");
        return 0;
    }

    printf("Habilidad: ");
    if (fgets(s.habilidad, sizeof(s.habilidad), stdin) == NULL) return 0;
    s.habilidad[strcspn(s.habilidad, "\n")] = '\0';
    if (s.habilidad[0] == '\0') strcpy(s.habilidad, "ninguna");

    printf("Salud (0-100): ");
    if (scanf("%d", &s.salud) != 1) { limpiar_buffer(); return 0; }
    limpiar_buffer();
    if (s.salud < 0 || s.salud > 100) {
        printf("La salud debe estar entre 0 y 100.\n");
        return 0;
    }

    /* 3) Guardar y avanzar contador */
    r->lista[r->n] = s;
    r->n++;
    return 1;
}

int modificar_recurso(Refugio *r,int tipo, int cantidad) {
    if(cantidad == 0)return 0;
    int *p = NULL;

    switch (tipo) {
        case 0: p =&r->recursos.agua; break;
        case 1: p =&r->recursos.comida; break;
        case 2: p =&r->recursos.medicamentos; break;
        case 3: p =&r->recursos.municiones; break;
        default: return 0;

    }
    if (cantidad < 0) {
        int salida = -cantidad;
        if (*p < salida)return 0;
        *p -= salida;
    }else{
        *p += cantidad;
    }
    return 1;
} 

    void listar_supervivientes(const Refugio *r) {
        if (r->n == 0) {
            printf("No hay supervivientes.\n");
            return;
        }

        for (int i = 0; i < r->n; i++) {    
            printf("%d) %s | %d | %s | %d\n", 
                i +1, 
                r->lista [i].nombre,
                r->lista [i].edad, 
                r->lista [i].habilidad,
                r->lista [i].salud);
        }
    }

    void mostrar_recursos(const Refugio *r);

    void mostrar_recursos(const Refugio *r) {
        printf("Recusos del Refugio: \n");
        printf("Agua: %d\n", r->recursos.agua);
        printf("Comida: %d\n", r->recursos.comida);
        printf("Medicamentos: %d\n", r->recursos.medicamentos);
         printf("Municiones: %d\n", r->recursos.municiones);
    }
    

int main (void) {
    Refugio refugio;



    /*Inicialización esplicita a "Vacío" */
    refugio.lista =NULL;
    refugio.n  = 0;
    refugio.capacidad = 0;

    /* Hu-04: Valores inicciales documentales */
    refugio.recursos.agua =50;
    refugio.recursos.comida =50;
    refugio.recursos.medicamentos =50;
    refugio.recursos.municiones =50;

    mostrar_recursos(&refugio);

   /* HU-05: pruebas */
    modificar_recurso(&refugio, 0, +10);    /* +10 agua       -> OK  */
    modificar_recurso(&refugio, 0, -60);    /* -60 > 50 + 10 -> rechazo */
    modificar_recurso(&refugio, 0, -60);    /* ahora hay 60   -> OK (queda 0) */
    modificar_recurso(&refugio, 0, -1);     /* salida > 0     -> rechazo */
    modificar_recurso(&refugio, 9, +5);     /* tipo inválido  -> rechazo */

    mostrar_recursos(&refugio);

    /* Preuba 1:Listar con lista vacia (no debe de fallar) */
    listar_supervivientes(&refugio);

    /*Pruebas manual: registrar 3 supervivientes */
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);
    listar_supervivientes(&refugio);

    printf("Refugio Inicializado: n=%d, capacidad=%d\n", refugio.n, refugio.capacidad);

    /* Liberar antes de salir */
    free(refugio.lista);
    refugio.lista = NULL;


    return 0;
}
