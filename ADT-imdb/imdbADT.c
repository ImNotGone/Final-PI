#include "imdbADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 1)
// Se dio a entender en una respuesta en el foro
// que no alcanzaba unicamente usar errno == ENOMEM
// por lo que comprendemos que hay casos en los que los 
// -alloc retornan NULL sin setear correctamente el errno (?)
// asi que decidimos hacerlo manualmente en cada chequeo 
// por las dudas, para que funcione correctamente el 
// errno == ENOMEM en el main.c luego de addData

// Cuantos bloques de memoria se reservan de una en 
// la funcion auxiliar "copy"
#define BLOCK 10

// Para validar un type
#define VALID_TYPE(type) ((type) >= 0 && (type) < CANT_TYPES)

// Para guardar la informacion relevante a la Q3
// tanto para peliculas como para series
typedef struct tMediaInfo {
    char * title;       
    size_t cantVotos;   
    float rating;
} tMediaInfo;

// Usamos una lista para poder insertar en orden alfabetico por genero
typedef struct tNGenre {
    char * genre;           // El genero del nodo actual
    size_t cant;            // La cantidad de apariciones de dicho genero para "TRACK_GENRE_TO"
    struct tNGenre * tail;  // Puntero al siguiente nodo en la lista
} tNGenre;

typedef tNGenre * tLGenre;

// Usamos una lista para insertar en orden decendente por anio
typedef struct tNYear {
    int year;                       // El anio actual
    size_t cant[CANT_TYPES];        // La cantidad de peliculas en la pos "MOVIE" y de series en la pos "SERIES"
    tLGenre first;                  // Puntero al primer nodo de la lista de generos para cada anio
    tMediaInfo media[CANT_TYPES];   // La informacion de la pelicula mas votada en la pos "MOVIE" y la serie mas votada en la pos "SERIES"
    struct tNYear * tail;           // Puntero al siguiente nodo de la lista
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;   // puntero al primero nodo de la lista de anios
    tLYear iterY;   // puntero auxiliar para iterar por los anios
    tLGenre iterG;  // puntero auxiliar para iterar por los generos de un anio "X"
} imdbCDT;

// funcion auxiliar que determina el orden en el cual se guardan los anios
static int compareYear(int year1, int year2) {
    return year2 - year1;
}

// funcion auxiliar que determina el orden en el cual se guardan los generos
static int compareGenre(char * genre1, char * genre2) {
    return strcmp(genre1, genre2);
}

imdbADT newImdb(void) {
    return calloc(1, sizeof(imdbCDT));
}

// Funcion auxiliar para liberar
// la memoria reservada para la listas de generos
static void freeGenre(tLGenre genre) {
    if (genre == NULL)
        return;
    freeGenre(genre->tail);
    free(genre->genre);
    free(genre);
}

// Funcion auxiliar para liberar 
// la memoria reservada para la lista de anios
static void freeYear(tLYear year) {
    if (year == NULL) 
        return;
    freeYear(year->tail);
    freeGenre(year->first);
    for(int i = 0; i < CANT_TYPES; i++) {
        free(year->media[i].title);
    }
    free(year);
}

void freeImdb(imdbADT imdb) {
    freeYear(imdb->first);
    free(imdb);
}

// Auxiliar copy(), recibe un string
// devuelve un puntero a la copia del string almacenada en el heap
// si no se pudo reservar memoria retorna NULL
// y se asegura que errno sea ENOMEM ver -> (1)
static char * copy(char * source) {
    int i;
    char * dest = NULL;
    for (i = 0; source[i] != '\0'; i++) {
        if (i % BLOCK == 0){
            dest = realloc(dest, (i + BLOCK)*sizeof(char));
            if (dest == NULL || errno == ENOMEM) {
                errno = ENOMEM; // ver -> (1)
                return NULL;
            }
        }
        dest[i] = source[i];
    }
    dest = realloc(dest, (i+1)*sizeof(char));
    if (dest == NULL || errno == ENOMEM) {
        errno = ENOMEM; // ver -> (1)
        return NULL;
    }
    dest[i] = '\0';
    return dest;
}

// Funcion auxiliar para cargar la nueva informacion
static void addMedia(tMediaInfo * media, char * newTitle, float newRating, size_t newVotes) {
    // Libera el titulo anterior, si no habia libera NULL (por eso usamos calloc en addToYear)
    free(media->title);
    // Genera el nuevo titulo
    media->title = copy(newTitle);
    if (media->title == NULL || errno == ENOMEM) {
        // errno = ENOMEM; no volvemos a setear errno en ENOMEM, ya que deberia venir seteado desde copy()
        return;
    }
    // Actualizo los datos de la pelicula/serie
    media->cantVotos = newVotes;
    media->rating = newRating;
}

// Funcion auxiliar para la carga de datos a un anio nuevo
// o la actualizacion de los datos de un anio que ya estaba
static tLYear addToYearRec(tLYear first, titleType type, char * title, int year, float rating, size_t votes) {
    int c;
    if (first == NULL || (c=compareYear(first->year, year)) > 0 ) {
        // Para que se inicie correctamente el vector de tMediaInfo y de cantidades
        // utilizamos calloc()
        tLYear newNode = calloc(1, sizeof(tNYear)); 
        if (newNode == NULL || errno == ENOMEM) {
            errno = ENOMEM; // ver -> (1)
            // Si no hay memoria quiero que se siga encadenando la lista 
            // por lo que retorno first
            // ademas no puedo desreferenciar el nuevo nodo
            // ya que es una posicion invalida de memoria
            return first; 
        }
        newNode->year = year;
        newNode->tail = first;
        newNode->cant[type] = 1;
        addMedia(&newNode->media[type], title, rating, votes);
        return newNode;
    }
    if (c == 0) {
        // Incremento el contador del tipo especifico
        first->cant[type]++;
        // Si el actual tiene menos votos que el entrante
        // se actualiza el actual para que siempre quede aquel que mas votos tiene
        if (votes > first->media[type].cantVotos) {
            addMedia(&first->media[type], title, rating, votes);
        }
        return first;
    }
    first->tail = addToYearRec(first->tail, type, title, year, rating, votes);
    return first;
}

// Funcion auxiliar para la carga de datos a un genero nuevo
// o la actualizacion de la cantidad por genero si el mismo ya existe
static tLGenre addToGenreRec(tLGenre first, char * genre) {
    int c;
    if (first == NULL || (c=compareGenre(first->genre, genre)) > 0 ) {
        // malloc() ya que voy a llenar todo el struct
        tLGenre newNode = malloc(sizeof(tNGenre)); 
        if (newNode == NULL || errno == ENOMEM) {
            errno = ENOMEM; 
            // Si no hay memoria quiero que se siga encadenando la lista 
            // por lo que retorno first
            // ademas no puedo desreferenciar el nuevo nodo
            // ya que es una posicion invalida de memoria
            return first;
        }
        if ((newNode->genre = copy(genre)) == NULL || errno == ENOMEM) {
            // errno = ENOMEM; no volvemos a setear errno en ENOMEM, ya que deberia venir seteado desde copy()
            // en caso de que falle la copia del nombre del nuevo genero
            // libero la memoria reservada por el nuevo nodo
            free(newNode);
            // Si no hay memoria quiero que se siga encadenando la lista 
            // por lo que retorno first
            return first;
        }
        newNode->cant = 1;
        newNode->tail = first;
        return newNode;
    }
    if (c == 0) {
        // Incremento el contador para este genero
        first->cant++;
        return first;
    }
    first->tail = addToGenreRec(first->tail, genre);
    return first;
}

// Funcion auxiliar para la busqueda de un anio en la lista
// si el anio no esta retorna NULL
static tLYear searchYear(tLYear first, int year) {
    int c;
    if (first == NULL || (c = compareYear(first->year, year)) > 0)
        return NULL;
    if (c == 0)
        return first;
    return searchYear(first->tail, year);
    /*
    tLYear iter = first;
    while(iter != NULL && compareYear(iter->year, year) < 0) {
        iter = iter->tail;
    }
    if (iter == NULL || compareYear(iter->year, year) != 0)
        return NULL;
    return iter;
    */
}

int addToGenre(imdbADT imdb, char * genre, int year) {
    // Busco el anio en la lista
    tLYear currentYear = searchYear(imdb->first, year);
    // Si el anio no estaba retorno "EYEAR"
    if (currentYear == NULL) {
        return EYEAR;
    }
    // Sino actualizo la lista de generos en el TAD
    currentYear->first = addToGenreRec(currentYear->first, genre);
    // Si hubo algun error al reservar memoria 
    // retorno errno (que ya vino de addToGenreRec() seteado en "ENOMEM")
    if (errno == ENOMEM) 
        return errno;
    return OK;
}

int addToYear(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes) {
    // Si el tipo no es valido retorno !"OK"
    if (!VALID_TYPE(type)) {
        return !OK;
    }
    imdb->first = addToYearRec(imdb->first, type, title, year, rating, votes);
    // Si hubo algun error al reservar memoria
    // retorno errno (que ya vino de addToGenreRec() seteado en "ENOMEM")
    if (errno == ENOMEM)
        return errno;
    return OK;
}

void toBeginYear(imdbADT imdb) {
    imdb->iterY = imdb->first;
}

int hasNextYear(imdbADT imdb) {
    return imdb->iterY != NULL;
}

int nextYear(imdbADT imdb) {
    if (!hasNextYear(imdb))
        return ENEXT;
    imdb->iterY = imdb->iterY->tail;
    return OK;
}

void toBeginGenre(imdbADT imdb, int year){
    tLYear yearNode = searchYear( imdb->first , year);
    // si searchYear me devolvio NULL el anio no estaba
    // por lo que dejo el iterador en NULL para que hasnext
    // no permita iterar
    if (yearNode == NULL) {
        imdb->iterG = NULL;
    } else {
        imdb->iterG = yearNode->first;
    }
}

int hasNextGenre(imdbADT imdb){
    return imdb->iterG != NULL;
}

int nextGenre(imdbADT imdb){
    if (!hasNextGenre(imdb))
        return ENEXT;
    imdb->iterG = imdb->iterG->tail;
    return OK;
}

int getYear(imdbADT imdb) {
    if (imdb->iterY == NULL)
        return EYEAR;
    return imdb->iterY->year;
}

size_t getCant(imdbADT imdb, titleType type) {
    if (!VALID_TYPE(type) || imdb->iterY == NULL) 
        return ECANT;
    return imdb->iterY->cant[type];
}

char * getTitle(imdbADT imdb, titleType type) {
    if (!VALID_TYPE(type) || imdb->iterY == NULL) {
        errno = EINVAL;
        return NULL;
    }
    // Si no habia titulo para ese "type" lo remplazo por NONE
    // De esta manera puedo copiar correctamente y en el .csv me queda "\N"
    // para el campo vacio.
    // Esto puede pasar por ejemplo si hay una pelicula 
    // pero no una serie para un anio o viceversa
    char * title;
    if (imdb->iterY->media[type].title != NULL) 
        title = copy(imdb->iterY->media[type].title);
    else
        title = copy(NONE);
    if (title == NULL || errno == ENOMEM) {
        // errno = ENOMEM; no volvemos a setear errno en ENOMEM, ya que deberia venir seteado desde copy()
        return NULL;
    }
    return title;
}

size_t getVotes(imdbADT imdb, titleType type) {
    if (!VALID_TYPE(type) || imdb->iterY == NULL) 
        return ECANT;
    return imdb->iterY->media[type].cantVotos;
}

float getRating(imdbADT imdb, titleType type) {
    if (!VALID_TYPE(type) || imdb->iterY == NULL) 
        return ECANT;
    return imdb->iterY->media[type].rating;
}

char * getGenre(imdbADT imdb) {
    if (imdb->iterG == NULL) 
        return NULL;
    char * genre = copy(imdb->iterG->genre);
    if (genre == NULL || errno == ENOMEM) {
        // errno = ENOMEM; no volvemos a setear errno en ENOMEM, ya que deberia venir seteado desde copy()
        return NULL;
    }
    return genre;
}

size_t getCantGenre(imdbADT imdb) {
    if(imdb->iterG == NULL)
        return ECANT;
    return imdb->iterG->cant;
}