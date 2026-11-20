#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOMBRE    30
#define MAX_HABILIDAD 30
#define MAX_DESC      100
#define CAP_INICIAL   4

/* HU-10: nombres de archivos */
#define ARCH_SUP   "supervivientes.txt"
#define ARCH_REC   "recursos.txt"
#define ARCH_HIS   "historial.txt"
#define TMP_SUP    "supervivientes.tmp"
#define TMP_REC    "recursos.tmp"
#define TMP_HIS    "historial.tmp"

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
void perder_recurso(Refugio *r, int tipo, int cantidad);
int  guardar_partida(const Refugio *r);
int  cargar_partida(Refugio *r);
void liberar_refugio(Refugio *r);        /* HU-13, usada por HU-11 */
void evento_aleatorio(Refugio *r);       /* HU-12 */

/* ============================================================
 * Utilidad: limpiar buffer
 * ============================================================ */
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* ============================================================
 * HU-13: Liberar toda la memoria del refugio
 * ============================================================ */
void liberar_refugio(Refugio *r) {
    free(r->lista);
    free(r->historial);
    r->lista       = NULL;
    r->historial   = NULL;
    r->n           = 0;
    r->capacidad   = 0;
    r->n_eventos   = 0;
    r->cap_eventos = 0;
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
 * HU-10: Guardar partida de forma segura
 * ============================================================ */
int guardar_partida(const Refugio *r) {
    /* --- 1) Supervivientes --- */
    FILE *f = fopen(TMP_SUP, "w");
    if (f == NULL) {
        printf("No se pudo abrir %s para escritura.\n", TMP_SUP);
        return 0;
    }
    for (int i = 0; i < r->n; i++) {
        if (fprintf(f, "%s|%d|%s|%d\n",
                    r->lista[i].nombre,
                    r->lista[i].edad,
                    r->lista[i].habilidad,
                    r->lista[i].salud) < 0) {
            fclose(f);
            return 0;
        }
    }
    if (fclose(f) != 0) return 0;

    /* --- 2) Recursos --- */
    f = fopen(TMP_REC, "w");
    if (f == NULL) {
        printf("No se pudo abrir %s para escritura.\n", TMP_REC);
        return 0;
    }
    if (fprintf(f, "%d|%d|%d|%d\n",
                r->recursos.agua,
                r->recursos.comida,
                r->recursos.medicamentos,
                r->recursos.municiones) < 0) {
        fclose(f);
        return 0;
    }
    if (fclose(f) != 0) return 0;

    /* --- 3) Historial --- */
    f = fopen(TMP_HIS, "w");
    if (f == NULL) {
        printf("No se pudo abrir %s para escritura.\n", TMP_HIS);
        return 0;
    }
    if (fprintf(f, "%d\n", r->dia_actual) < 0) {
        fclose(f);
        return 0;
    }
    for (int i = 0; i < r->n_eventos; i++) {
        if (fprintf(f, "%d|%s\n",
                    r->historial[i].dia,
                    r->historial[i].descripcion) < 0) {
            fclose(f);
            return 0;
        }
    }
    if (fclose(f) != 0) return 0;

    /* --- 4) Renombrar a definitivos --- */
    if (rename(TMP_SUP, ARCH_SUP) != 0) return 0;
    if (rename(TMP_REC, ARCH_REC) != 0) return 0;
    if (rename(TMP_HIS, ARCH_HIS) != 0) return 0;

    return 1;
}

/* ============================================================
 * HU-11: Cargar partida (corregida)
 * - Libera lo reservado si falla
 * - Avisa si hay líneas dañadas
 * - Libera lo anterior al inicio
 * ============================================================ */
int cargar_partida(Refugio *r) {
    /* Liberar lo que hubiera antes (por si se llama dos veces) */
    liberar_refugio(r);
    r->dia_actual = 1;

    /* --- 1) Recursos --- */
    FILE *f = fopen(ARCH_REC, "r");
    if (f == NULL) {
        /* primera ejecución: no hay archivos, no es error */
        return 1;
    }

    int a, c, m, mu;
    if (fscanf(f, "%d|%d|%d|%d", &a, &c, &m, &mu) != 4) {
        fclose(f);
        printf("Archivo de recursos danado. Se ignora la carga.\n");
        return 0;
    }
    fclose(f);

    if (a < 0 || c < 0 || m < 0 || mu < 0) {
        printf("Recursos fuera de rango. Se ignora la carga.\n");
        return 0;
    }
    r->recursos.agua         = a;
    r->recursos.comida       = c;
    r->recursos.medicamentos = m;
    r->recursos.municiones   = mu;

    /* --- 2) Supervivientes --- */
    f = fopen(ARCH_SUP, "r");
    if (f != NULL) {
        char linea[256];
        while (fgets(linea, sizeof(linea), f) != NULL) {
            linea[strcspn(linea, "\n")] = '\0';
            if (linea[0] == '\0') continue;

            char nom[MAX_NOMBRE + 1];
            char hab[MAX_HABILIDAD + 1];
            int  edad, salud;

            if (sscanf(linea, "%30[^|]|%d|%30[^|]|%d",
                       nom, &edad, hab, &salud) != 4) {
                printf("Linea de superviviente danada, se ignora.\n");
                continue;
            }
            if (edad <= 0 || salud < 0 || salud > 100) {
                printf("Superviviente fuera de rango (%s), se ignora.\n", nom);
                continue;
            }

            if (r->n == r->capacidad) {
                int nueva_cap = (r->capacidad == 0) ? CAP_INICIAL
                                                    : r->capacidad * 2;
                Superviviente *tmp = realloc(r->lista,
                                     (size_t)nueva_cap * sizeof(Superviviente));
                if (tmp == NULL) {
                    fclose(f);
                    printf("Error de memoria al cargar supervivientes.\n");
                    liberar_refugio(r);
                    return 0;
                }
                r->lista = tmp;
                r->capacidad = nueva_cap;
            }

            strncpy(r->lista[r->n].nombre, nom, MAX_NOMBRE);
            r->lista[r->n].nombre[MAX_NOMBRE] = '\0';
            r->lista[r->n].edad = edad;
            strncpy(r->lista[r->n].habilidad, hab, MAX_HABILIDAD);
            r->lista[r->n].habilidad[MAX_HABILIDAD] = '\0';
            r->lista[r->n].salud = salud;
            r->n++;
        }
        fclose(f);
    }

    /* --- 3) Historial --- */
    f = fopen(ARCH_HIS, "r");
    if (f != NULL) {
        char linea[512];

        if (fgets(linea, sizeof(linea), f) != NULL) {
            int dia;
            if (sscanf(linea, "%d", &dia) == 1 && dia >= 0) {
                r->dia_actual = dia;
            }
        }

        while (fgets(linea, sizeof(linea), f) != NULL) {
            linea[strcspn(linea, "\n")] = '\0';
            if (linea[0] == '\0') continue;

            int  dia;
            char desc[MAX_DESC + 1];
            if (sscanf(linea, "%d|%100[^\n]", &dia, desc) != 2) {
                printf("Linea de historial danada, se ignora.\n");
                continue;
            }

            if (r->n_eventos == r->cap_eventos) {
                int nueva_cap = (r->cap_eventos == 0) ? CAP_INICIAL
                                                      : r->cap_eventos * 2;
                Evento *tmp = realloc(r->historial,
                                      (size_t)nueva_cap * sizeof(Evento));
                if (tmp == NULL) {
                    fclose(f);
                    printf("Error de memoria al cargar historial.\n");
                    liberar_refugio(r);
                    return 0;
                }
                r->historial = tmp;
                r->cap_eventos = nueva_cap;
            }

            r->historial[r->n_eventos].dia = dia;
            strncpy(r->historial[r->n_eventos].descripcion, desc, MAX_DESC);
            r->historial[r->n_eventos].descripcion[MAX_DESC] = '\0';
            r->n_eventos++;
        }
        fclose(f);
    }

    return 1;
}

/* ============================================================
 * HU-12: Evento aleatorio (5 tipos)
 * ============================================================ */
void evento_aleatorio(Refugio *r) {
    int tipo = rand() % 5;
    char texto[MAX_DESC + 1];

    switch (tipo) {
        case 0: { /* ¡Llegó un médico! */
            if (r->n == r->capacidad) {
                int nueva_cap = (r->capacidad == 0) ? CAP_INICIAL
                                                    : r->capacidad * 2;
                Superviviente *tmp = realloc(r->lista,
                                     (size_t)nueva_cap * sizeof(Superviviente));
                if (tmp == NULL) {
                    printf("Error de memoria al agregar medico.\n");
                    return;
                }
                r->lista = tmp;
                r->capacidad = nueva_cap;
            }

            Superviviente s;
            snprintf(s.nombre, sizeof(s.nombre), "Medico_%d", r->n + 1);
            s.edad = 30 + rand() % 30;
            strcpy(s.habilidad, "medicina");
            s.salud = 100;

            r->lista[r->n] = s;
            r->n++;

            snprintf(texto, sizeof(texto),
                     "Evento: llego un medico (%s)", s.nombre);
            registrar_evento(r, texto);
            printf("[Evento] Llego un medico: %s\n", s.nombre);
            break;
        }

        case 1: { /* La horda atacó */
            int m = 5 + rand() % 10;
            perder_recurso(r, 3, m);
            perder_recurso(r, 1, 3);

            snprintf(texto, sizeof(texto),
                     "Evento: la horda ataco (-%d municiones, -3 comida)", m);
            registrar_evento(r, texto);
            printf("[Evento] La horda ataco.\n");
            break;
        }

        case 2: { /* Alguien robó la comida */
            int c = 2 + rand() % 8;
            perder_recurso(r, 1, c);

            snprintf(texto, sizeof(texto),
                     "Evento: alguien robo %d de comida", c);
            registrar_evento(r, texto);
            printf("[Evento] Alguien robo comida.\n");
            break;
        }

        case 3: { /* Encontraron una mochila */
            modificar_recurso(r, 0, 5);
            modificar_recurso(r, 2, 2);

            snprintf(texto, sizeof(texto),
                     "Evento: encontraron mochila (+5 agua, +2 medicamentos)");
            registrar_evento(r, texto);
            printf("[Evento] Encontraron una mochila.\n");
            break;
        }

        case 4: { /* ¡Se fue la luz! */
            printf("[Evento] Se fue la luz: guardando y recargando...\n");
            if (!guardar_partida(r)) {
                printf("No se pudo guardar durante el apagon.\n");
                return;
            }
            liberar_refugio(r);
            if (!cargar_partida(r)) {
                printf("No se pudo recargar tras el apagon.\n");
            }

            snprintf(texto, sizeof(texto),
                     "Evento: se fue la luz (partida guardada y recargada)");
            registrar_evento(r, texto);
            printf("[Evento] Partida guardada y recargada.\n");
            break;
        }
    }
}

/* ============================================================
 * main: menú interactivo
 * ============================================================ */
int main(void) {
    Refugio refugio;
    int opcion;

    /* Inicialización */
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

    /* HU-11: cargar partida al iniciar */
    cargar_partida(&refugio);

    /* Prueba opcional: 50 eventos seguidos
     * Ejecutar con: PRUEBA_50=1 ./refugio
     */
    if (getenv("PRUEBA_50") != NULL) {
        printf("--- Prueba de 50 eventos ---\n");
        for (int i = 0; i < 50; i++) {
            evento_aleatorio(&refugio);
        }
        if (refugio.recursos.agua < 0 || refugio.recursos.comida < 0 ||
            refugio.recursos.medicamentos < 0 || refugio.recursos.municiones < 0) {
            printf("ERROR: algun recurso negativo.\n");
        } else {
            printf("OK: ningun recurso negativo tras 50 eventos.\n");
        }
        mostrar_recursos(&refugio);
        liberar_refugio(&refugio);
        return 0;
    }

    do {
        printf("\n===== REFUGIO DE SUPERVIVIENTES ZOMBI =====\n");
        printf(" 1. Registrar superviviente      (HU-02)\n");
        printf(" 2. Listar supervivientes        (HU-03)\n");
        printf(" 3. Mostrar recursos             (HU-04)\n");
        printf(" 4. Modificar recurso            (HU-05)\n");
        printf(" 5. Ver historial                (HU-06)\n");
        printf(" 6. Dar de baja a superviviente  (HU-07)\n");
        printf(" 7. Simular expedicion           (HU-08)\n");
        printf(" 8. Registrar perdida de recurso (HU-09)\n");
        printf(" 9. Evento aleatorio             (HU-12)\n");
        printf("10. Guardar partida              (HU-10)\n");
        printf("11. Avanzar dia\n");
        printf(" 0. Salir (guarda y libera)\n");
        printf("Opcion: ");

        if (scanf("%d", &opcion) != 1) {
            limpiar_buffer();
            printf("Opcion invalida.\n");
            continue;
        }
        limpiar_buffer();

        switch (opcion) {
            case 1:
                agregar_superviviente(&refugio);
                break;

            case 2:
                listar_supervivientes(&refugio);
                break;

            case 3:
                mostrar_recursos(&refugio);
                break;

            case 4: {
                int tipo, cantidad;
                printf("Tipo (0=agua,1=comida,2=medicamentos,3=municiones): ");
                if (scanf("%d", &tipo) != 1) { limpiar_buffer(); break; }
                printf("Cantidad (+ entra, - sale): ");
                if (scanf("%d", &cantidad) != 1) { limpiar_buffer(); break; }
                limpiar_buffer();

                if (!modificar_recurso(&refugio, tipo, cantidad))
                    printf("Operacion rechazada.\n");
                else
                    printf("Recurso actualizado.\n");
                break;
            }

            case 5:
                mostrar_historial(&refugio);
                break;

            case 6: {
                char nombre[MAX_NOMBRE + 1];
                printf("Nombre a dar de baja: ");
                if (fgets(nombre, sizeof(nombre), stdin) == NULL) break;
                nombre[strcspn(nombre, "\n")] = '\0';
                if (!dar_de_baja(&refugio, nombre))
                    printf("No se encontro a '%s'.\n", nombre);
                else
                    printf("Baja registrada.\n");
                break;
            }

            case 7:
                expedicion(&refugio);
                break;

            case 8: {
                int tipo, cantidad;
                printf("Tipo (0=agua,1=comida,2=medicamentos,3=municiones): ");
                if (scanf("%d", &tipo) != 1) { limpiar_buffer(); break; }
                printf("Cantidad perdida: ");
                if (scanf("%d", &cantidad) != 1) { limpiar_buffer(); break; }
                limpiar_buffer();
                perder_recurso(&refugio, tipo, cantidad);
                break;
            }

            case 9:
                evento_aleatorio(&refugio);
                break;

            case 10:
                if (guardar_partida(&refugio))
                    printf("Partida guardada.\n");
                else
                    printf("No se pudo guardar.\n");
                break;

            case 11:
                refugio.dia_actual++;
                printf("Dia actual: %d\n", refugio.dia_actual);
                break;

            case 0:
                printf("Saliendo...\n");
                break;

            default:
                printf("Opcion invalida.\n");
                break;
        }

    } while (opcion != 0);

    /* HU-13: guardar y liberar al salir */
    guardar_partida(&refugio);
    liberar_refugio(&refugio);

    return 0;
}