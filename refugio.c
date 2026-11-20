#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOMBRE    30
#define MAX_HABILIDAD 30
#define MAX_DESC      100        /* ← ÚNICO CAMBIO: antes 60, ahora 100 */
#define CAP_INICIAL   4

/* ============================================================
 * Structs
 * ============================================================ */
typedef struct {
    char nombre[MAX_NOMBRE + 1];
    int  edad;
    char habilidad[MAX_HABILIDAD + 1];
    int  salud;                 /* 0 a 100 */
} Superviviente;

typedef struct {
    int agua;
    int comida;
    int medicamentos;
    int municiones;
} Recursos;

/* HU-06: Evento (día + descripción) */
typedef struct {
    int  dia;
    char descripcion[MAX_DESC + 1];
} Evento;

typedef struct {
    Superviviente *lista;
    int n;
    int capacidad;

    Recursos recursos;

    /* HU-06: historial dinámico */
    Evento *historial;
    int n_eventos;
    int cap_eventos;
    int dia_actual;
} Refugio;

/* ============================================================
 * Prototipos
 * ============================================================ */
int  agregar_superviviente(Refugio *r);
void listar_supervivientes(const Refugio *r);
void mostrar_recursos(const Refugio *r);
int  modificar_recurso(Refugio *r, int tipo, int cantidad);
int  registrar_evento(Refugio *r, const char *texto);
void mostrar_historial(const Refugio *r);
int  dar_de_baja(Refugio *r, const char *nombre);
int  expedicion(Refugio *r);
void perder_recurso(Refugio *r, int tipo, int cantidad);   /* HU-09 */

/* ============================================================
 * Utilidad: limpiar buffer
 * ============================================================ */
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* ============================================================
 * HU-02: Registrar superviviente
 * ============================================================ */
int agregar_superviviente(Refugio *r) {
    if (r->n == r->capacidad) {
        int nueva_cap = (r->capacidad == 0) ? CAP_INICIAL : r->capacidad * 2;
        Superviviente *tmp = realloc(r->lista,
                                     (size_t)nueva_cap * sizeof(Superviviente));
        if (tmp == NULL) {
            printf("Error: no hay memoria.\n");
            return 0;
        }
        r->lista = tmp;
        r->capacidad = nueva_cap;
    }

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

    r->lista[r->n] = s;
    r->n++;

    char texto[MAX_DESC + 1];
    snprintf(texto, sizeof(texto), "Llego superviviente: %s", s.nombre);
    registrar_evento(r, texto);

    return 1;
}

/* ============================================================
 * HU-03: Listar supervivientes
 * ============================================================ */
void listar_supervivientes(const Refugio *r) {
    if (r->n == 0) {
        printf("No hay supervivientes.\n");
        return;
    }
    printf("\n--- Supervivientes (%d) ---\n", r->n);
    for (int i = 0; i < r->n; i++) {
        printf("%d) %-30s | edad %3d | %-30s | salud %3d\n",
               i + 1,
               r->lista[i].nombre,
               r->lista[i].edad,
               r->lista[i].habilidad,
               r->lista[i].salud);
    }
}

/* ============================================================
 * HU-04: Mostrar recursos
 * ============================================================ */
void mostrar_recursos(const Refugio *r) {
    printf("\n--- Recursos del Refugio ---\n");
    printf("Agua.........: %d\n", r->recursos.agua);
    printf("Comida.......: %d\n", r->recursos.comida);
    printf("Medicamentos.: %d\n", r->recursos.medicamentos);
    printf("Municiones...: %d\n", r->recursos.municiones);
}

/* ============================================================
 * HU-05: Modificar recurso (validar ANTES de modificar)
 * ============================================================ */
int modificar_recurso(Refugio *r, int tipo, int cantidad) {
    if (cantidad == 0) return 0;

    int *p = NULL;
    switch (tipo) {
        case 0: p = &r->recursos.agua;         break;
        case 1: p = &r->recursos.comida;       break;
        case 2: p = &r->recursos.medicamentos; break;
        case 3: p = &r->recursos.municiones;   break;
        default: return 0;
    }

    if (cantidad < 0) {
        int salida = -cantidad;
        if (*p < salida) return 0;
        *p -= salida;
    } else {
        *p += cantidad;
    }
    return 1;
}

/* ============================================================
 * HU-06: Registrar un evento en el historial
 * ============================================================ */
int registrar_evento(Refugio *r, const char *texto) {
    if (r->n_eventos == r->cap_eventos) {
        int nueva_cap = (r->cap_eventos == 0) ? CAP_INICIAL : r->cap_eventos * 2;
        Evento *tmp = realloc(r->historial,
                              (size_t)nueva_cap * sizeof(Evento));
        if (tmp == NULL) {
            printf("Error: no hay memoria para el historial.\n");
            return 0;
        }
        r->historial = tmp;
        r->cap_eventos = nueva_cap;
    }

    r->historial[r->n_eventos].dia = r->dia_actual;

    strncpy(r->historial[r->n_eventos].descripcion, texto, MAX_DESC);
    r->historial[r->n_eventos].descripcion[MAX_DESC] = '\0';

    r->n_eventos++;
    return 1;
}

/* ============================================================
 * HU-06: Mostrar el historial
 * ============================================================ */
void mostrar_historial(const Refugio *r) {
    if (r->n_eventos == 0) {
        printf("El historial esta vacio.\n");
        return;
    }

    printf("\n--- Historial (%d eventos) ---\n", r->n_eventos);
    for (int i = 0; i < r->n_eventos; i++) {
        printf("[Dia %d] %s\n",
               r->historial[i].dia,
               r->historial[i].descripcion);
    }
}

/* ============================================================
 * HU-07: Dar de baja a un superviviente por nombre
 * ============================================================ */
int dar_de_baja(Refugio *r, const char *nombre) {
    int idx = -1;
    for (int i = 0; i < r->n; i++) {
        if (strcmp(r->lista[i].nombre, nombre) == 0) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        printf("No se encontro a '%s'.\n", nombre);
        return 0;
    }

    for (int i = idx; i < r->n - 1; i++) {
        r->lista[i] = r->lista[i + 1];
    }
    r->n--;

    if (r->capacidad > CAP_INICIAL && r->n < r->capacidad / 4) {
        int nueva_cap = r->capacidad / 2;
        Superviviente *tmp = realloc(r->lista,
                                     (size_t)nueva_cap * sizeof(Superviviente));
        if (tmp != NULL) {
            r->lista = tmp;
            r->capacidad = nueva_cap;
        }
    }

    char texto[MAX_DESC + 1];
    snprintf(texto, sizeof(texto), "Baja de superviviente: %s", nombre);
    registrar_evento(r, texto);

    printf("Superviviente '%s' dado de baja.\n", nombre);
    return 1;
}

/* ============================================================
 * HU-08: Simular una expedición
 * ============================================================ */
int expedicion(Refugio *r) {
    int vivos = 0;
    for (int i = 0; i < r->n; i++) {
        if (r->lista[i].salud > 0) {
            vivos++;
        }
    }
    if (vivos == 0) {
        printf("No hay supervivientes con salud para expedicionar.\n");
        return 0;
    }

    const int COSTO_MUNICIONES = 5;
    if (r->recursos.municiones < COSTO_MUNICIONES) {
        printf("No hay municiones suficientes (se necesitan %d).\n",
               COSTO_MUNICIONES);
        return 0;
    }

    modificar_recurso(r, 3, -COSTO_MUNICIONES);

    int g_agua = rand() % 10;
    int g_com  = rand() % 10;
    modificar_recurso(r, 0, g_agua);
    modificar_recurso(r, 1, g_com);

    int pick = rand() % vivos;
    int elegido = -1;
    int cuenta = 0;
    for (int i = 0; i < r->n; i++) {
        if (r->lista[i].salud > 0) {
            if (cuenta == pick) { elegido = i; break; }
            cuenta++;
        }
    }

    int perdida = 5 + rand() % 20;
    r->lista[elegido].salud -= perdida;
    if (r->lista[elegido].salud < 0) r->lista[elegido].salud = 0;

    char texto[MAX_DESC + 1];
    snprintf(texto, sizeof(texto),
             "Expedicion: +%d agua +%d comida; %s pierde %d salud",
             g_agua, g_com, r->lista[elegido].nombre, perdida);
    registrar_evento(r, texto);

    if (rand() % 100 < 20) {
        char nombre[MAX_NOMBRE + 1];
        strncpy(nombre, r->lista[elegido].nombre, MAX_NOMBRE);
        nombre[MAX_NOMBRE] = '\0';

        dar_de_baja(r, nombre);

        snprintf(texto, sizeof(texto),
                 "Durante la expedicion desaparecio: %s", nombre);
        registrar_evento(r, texto);
        printf("El superviviente %s desaparecio en la expedicion.\n", nombre);
    }

    return 1;
}

/* ============================================================
 * HU-09: Registrar pérdida de recursos (nunca negativo)
 * ============================================================ */
void perder_recurso(Refugio *r, int tipo, int cantidad) {
    if (cantidad <= 0) return;

    int *p = NULL;
    switch (tipo) {
        case 0: p = &r->recursos.agua;         break;
        case 1: p = &r->recursos.comida;       break;
        case 2: p = &r->recursos.medicamentos; break;
        case 3: p = &r->recursos.municiones;   break;
        default: return;
    }

    if (*p >= cantidad) {
        *p -= cantidad;
    } else {
        *p = 0;
    }

    char texto[MAX_DESC + 1];
    snprintf(texto, sizeof(texto),
             "Perdida de %d de %s (quedan %d)",
             cantidad,
             (tipo == 0) ? "agua" :
             (tipo == 1) ? "comida" :
             (tipo == 2) ? "medicamentos" : "municiones",
             *p);
    registrar_evento(r, texto);
}

/* ============================================================
 * main
 * ============================================================ */
int main(void) {
    Refugio refugio;

    refugio.lista     = NULL;
    refugio.n         = 0;
    refugio.capacidad = 0;

    refugio.recursos.agua         = 50;
    refugio.recursos.comida       = 50;
    refugio.recursos.medicamentos = 20;
    refugio.recursos.municiones   = 30;

    refugio.historial   = NULL;
    refugio.n_eventos   = 0;
    refugio.cap_eventos = 0;
    refugio.dia_actual  = 1;

    srand((unsigned)time(NULL));

    mostrar_recursos(&refugio);

    /* HU-05 */
    modificar_recurso(&refugio, 0, +10);
    modificar_recurso(&refugio, 0, -60);
    modificar_recurso(&refugio, 0, -1);
    modificar_recurso(&refugio, 9, +5);

    mostrar_recursos(&refugio);

    /* HU-03 */
    listar_supervivientes(&refugio);

    /* HU-02 */
    printf("\n--- Registrando 3 supervivientes ---\n");
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);

    listar_supervivientes(&refugio);

    printf("\nEstado final: n=%d, capacidad=%d\n",
           refugio.n, refugio.capacidad);

    /* HU-06 */
    printf("\n--- Probando HU-06 ---\n");
    registrar_evento(&refugio, "Inicio de la partida");
    registrar_evento(&refugio, "Se registro un superviviente");
    mostrar_historial(&refugio);

    /* HU-07 */
    printf("\n--- Probando HU-07 ---\n");

    printf("\n[Caso 1] Baja del PRIMERO (Ana):\n");
    dar_de_baja(&refugio, "Ana");
    listar_supervivientes(&refugio);

    printf("\n[Caso 2] Baja del INTERMEDIO (Beto):\n");
    dar_de_baja(&refugio, "Beto");
    listar_supervivientes(&refugio);

    printf("\n[Caso 3] Baja del ULTIMO (Carla):\n");
    dar_de_baja(&refugio, "Carla");
    listar_supervivientes(&refugio);

    printf("\n[Caso 4] Baja del UNICO (no existe):\n");
    dar_de_baja(&refugio, "Nadie");
    listar_supervivientes(&refugio);

    mostrar_historial(&refugio);

    /* HU-08 */
    printf("\n--- Probando HU-08 ---\n");

    printf("\n[Setup] Registrando 3 nuevos supervivientes:\n");
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);
    agregar_superviviente(&refugio);
    listar_supervivientes(&refugio);
    mostrar_recursos(&refugio);

    printf("\n[Expedicion 1] Con municiones suficientes:\n");
    expedicion(&refugio);
    listar_supervivientes(&refugio);
    mostrar_recursos(&refugio);

    printf("\n[Setup] Gastando municiones a 0:\n");
    modificar_recurso(&refugio, 3, -refugio.recursos.municiones);
    mostrar_recursos(&refugio);

    printf("\n[Expedicion 2] Sin municiones (debe rechazar):\n");
    expedicion(&refugio);
    listar_supervivientes(&refugio);
    mostrar_recursos(&refugio);

    mostrar_historial(&refugio);

    /* HU-09 */
    printf("\n--- Probando HU-09 ---\n");

    refugio.recursos.agua         = 50;
    refugio.recursos.comida       = 50;
    refugio.recursos.medicamentos = 20;
    refugio.recursos.municiones   = 30;
    mostrar_recursos(&refugio);

    printf("\n[Perdida 1] Menor a lo disponible (agua -10, hay 50):\n");
    perder_recurso(&refugio, 0, 10);
    mostrar_recursos(&refugio);

    printf("\n[Perdida 2] Igual a lo disponible (agua -40, hay 40):\n");
    perder_recurso(&refugio, 0, 40);
    mostrar_recursos(&refugio);

    printf("\n[Perdida 3] Mayor a lo disponible (agua -99, hay 0):\n");
    perder_recurso(&refugio, 0, 99);
    mostrar_recursos(&refugio);

    printf("\n[Perdida 4] Cantidad inválida (agua -5):\n");
    perder_recurso(&refugio, 0, -5);
    mostrar_recursos(&refugio);

    printf("\n[Perdida 5] Tipo inválido (tipo 9):\n");
    perder_recurso(&refugio, 9, 5);
    mostrar_recursos(&refugio);

    mostrar_historial(&refugio);

    /* HU-13 (parcial) */
    free(refugio.lista);
    free(refugio.historial);
    refugio.lista     = NULL;
    refugio.historial = NULL;

    return 0;
}