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
    size_t cant[CANT_TYPES];        // La cantidad de peliculas en la pos "MOVIE" (=0) y de series en la pos "SERIES" (=1) 
    tLGenre first;                  // Puntero al primer nodo de la lista de generos para cada anio
    tMediaInfo media[CANT_TYPES];   // La informacion de la pelicula mas votada en la pos "MOVIE" (=0) y la serie mas votada en la pos "SERIES" (=1) 
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
static tLYear addToYear(tLYear first, titleType type, char * title, int year, float rating, size_t votes) {
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
    first->tail = addToYear(first->tail, type, title, year, rating, votes);
    return first;
}

// Funcion auxiliar para la carga de datos a un genero nuevo
// o la actualizacion de la cantidad por genero si el mismo ya existe
static tLGenre addToGenre(tLGenre first, char * genre) {
    int c;
    if (first == NULL || (c=compareGenre(first->genre, genre)) > 0 ) {
        // malloc() ya que voy a llenar todo el struct
        tLGenre newNode = malloc(sizeof(tNGenre)); 
        if (newNode == NULL || errno == ENOMEM) {
            // errno = ENOMEM; no volvemos a setear errno en ENOMEM, ya que deberia venir seteado desde copy()
            // Si no hay memoria quiero que se siga encadenando la lista 
            // por lo que retorno first
            // ademas no puedo desreferenciar el nuevo nodo
            // ya que es una posicion invalida de memoria
            return first;
        }
        if ((newNode->genre = copy(genre)) == NULL || errno == ENOMEM) {
            errno = ENOMEM; // ver -> (1)
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
    first->tail = addToGenre(first->tail, genre);
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

int addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres) {
    // Si el tipo no es valido retorno !"OK"
    if (type < 0 || type >= CANT_TYPES) {
        return !OK;
    }
    imdb->first = addToYear(imdb->first, type, title, year, rating, votes);
    // Si hubo algun error al reservar memoria lo atrapo
    // y retorno lo antes posible para que se notifique al usuario
    if (errno == ENOMEM)
        return errno;
    if (type == TRACK_GENRE_TO) {
        char * genre;
        // Agregue el anio en addToYear asi que no verifico que no este :)
        tLYear currentYear = searchYear(imdb->first, year);
        for (genre = strtok(genres, DELIM_GENRE); genre != NULL; genre = strtok(NULL, DELIM_GENRE)) {
            currentYear->first = addToGenre(currentYear->first, genre);
            // Si hubo algun error al reservar memoria lo atrapo
            // y retorno lo antes posible para que se notifique al usuario
            if (errno == ENOMEM)
                return errno;
        }
    }
    return OK;
}

void toBeginYear(imdbADT imdb) {
    imdb->iterY = imdb->first;
}

int hasNextYear(imdbADT imdb) {
    return imdb->iterY != NULL;
}

int nextYear(imdbADT imdb) {
    if(!hasNextYear(imdb))
        return ENEXT;
    int aux = imdb->iterY->year;
    imdb->iterY = imdb->iterY->tail;
    return aux;
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
    if(!hasNextGenre(imdb))
        return ENEXT;
    imdb->iterG = imdb->iterG->tail;
    return OK;
}

int getQ1(imdbADT imdb, char * buff) {
    int year = imdb->iterY->year;
    size_t films = imdb->iterY->cant[MOVIE];
    size_t series = imdb->iterY->cant[SERIES];
    return sprintf(buff, "%d%s%zu%s%zu", year, DELIM ,films, DELIM, series);
}

int getQ2(imdbADT imdb, char * buff, int year) {
    size_t films = imdb->iterG->cant;
    char * genre = imdb->iterG->genre;
    return sprintf(buff, "%d%s%s%s%zu", year, DELIM, genre, DELIM, films);
}

int getQ3(imdbADT imdb, char * buff) {
    int year = imdb->iterY->year;
    char * film = imdb->iterY->media[MOVIE].title;
    if (film == NULL)
        film = NONE; 
    size_t votesFilm = imdb->iterY->media[MOVIE].cantVotos;
    float ratingFilm = imdb->iterY->media[MOVIE].rating;
    char * serie = imdb->iterY->media[SERIES].title;
    if (serie == NULL)
        serie = NONE;
    size_t votesSerie = imdb->iterY->media[SERIES].cantVotos;
    float ratingSeries = imdb->iterY->media[SERIES].rating;
    return sprintf(buff, "%d%s%s%s%zu%s%.1f%s%s%s%zu%s%.1f", year, DELIM, film, DELIM, votesFilm, DELIM, ratingFilm, DELIM, serie, DELIM, votesSerie, DELIM, ratingSeries);
}