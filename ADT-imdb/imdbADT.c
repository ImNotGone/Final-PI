#include "imdbADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct tMediaInfo {
    char * title;
    size_t cantVotos;
    float rating;
} tMediaInfo;

typedef struct tNGenre {
    char * genre;
    size_t cantFilms;
    struct tNGenre * tail;
} tNGenre;

typedef tNGenre * tLGenre;

typedef struct tNYear {
    int year;
    size_t cant[CANT_TYPES]; // La pelicula en la pos. 0 y la serie en la pos. 1 
    tLGenre first;
    tMediaInfo media[CANT_TYPES]; // La pelicula en la pos. 0 y la serie en la pos. 1 
    struct tNYear * tail; 
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;
    tLYear iterY;
    tLGenre iterG;
} imdbCDT;

static int compareYear(int year1, int year2) {
    return year2 - year1;
}

static int compareGenre(char * genre1, char * genre2) {
    return strcmp(genre1, genre2);
}

imdbADT newImdb() { 
    return calloc(1, sizeof(imdbCDT));
}

static void freeGenre(tLGenre genre) {
    if (genre == NULL)
        return;
    freeGenre(genre->tail);
    free(genre->genre);
    free(genre);
}

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

static char * copy (char * source) {
    int i;
    char * dest = NULL;
    for (i = 0; source[i] != '\0'; i++) {
        if (i % BLOCK == 0){
            dest = realloc(dest, (i + BLOCK)*sizeof(char)); // sizeof por claridad aunque no hace falta (1)
            if (dest == NULL || errno == ENOMEM)
                return NULL;
        }
        dest[i] = source[i];
    }
    dest = realloc(dest, (i+1)*sizeof(char)); // idem (1)
    if (dest == NULL || errno == ENOMEM)
        return NULL;
    dest[i] = '\0';
    return dest;
}

static void addMedia( tMediaInfo * media , char * newTitle , float newRating , size_t newVotes) {
    free(media->title); // si es NULL no pasa nada, sino lo libera el titulo anterior
    media->title = copy(newTitle);
    if (media->title == NULL || errno == ENOMEM)
        return ;
    media->cantVotos = newVotes;
    media->rating = newRating;
}

static tLYear addToYear(tLYear first, titleType type, char * title, int year, float rating, size_t votes) {
    int c;
    if (first == NULL || (c=compareYear(first->year, year)) > 0 ) {
        tLYear aux = calloc(1, sizeof(tNYear)); // para que se inicie correctamente el vector de tMediaInfo y de cantidades
        if (aux == NULL || errno == ENOMEM){
            return first; // Si no hay memoria quiero que se siga encadenando la lista y ademas no puedo desreferenciar aux 
        }
        aux->year = year;
        aux->tail = first;
        aux->cant[type] = 1;
        addMedia(&aux->media[type], title, rating, votes);
        return aux;
    }
    if (c == 0){
        first->cant[type]++;
        if (votes > first->media[type].cantVotos) {
            addMedia(&first->media[type], title, rating, votes);
        }
        return first;
    }
    first->tail = addToYear(first->tail, type, title, year, rating, votes);
    return first;
}

static tLGenre addToGenreRec(tLGenre first, char * genre) {
    int c;
    if (first == NULL || (c=compareGenre(first->genre, genre)) > 0 ) {
        tLGenre aux = malloc(sizeof(tNGenre)); // malloc ya que voy a llenar toda la struct
        if (aux == NULL || errno == ENOMEM){
            return first; // Si no hay memoria quiero que se siga encadenando la lista y ademas no puedo desreferenciar aux 
        }
        aux->genre = copy(genre);
        aux->cantFilms = 1;
        aux->tail = first;
        return aux; // retornaba first, 30 minutos de agonia y 13 millones de bytes leakeados B-)
        // life is pain
    }
    if (c == 0) {
        first->cantFilms++;
        return first;
    }
    first->tail = addToGenreRec(first->tail, genre);
    return first;
}

static tLYear searchYear(tLYear first, int year) {
    // no chequeo que no este "year" en la lista, ya que deberia haberse agregado en addToYear
    if (compareYear(first->year, year) == 0) {
        return first;
    }
    return searchYear(first->tail, year);
}

static void addToGenre(tLYear first, int year, char * genre) {
    tLYear cYear = searchYear(first, year);
    cYear->first = addToGenreRec(cYear->first, genre);
}

int addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres) {
    imdb->first = addToYear(imdb->first, type, title, year, rating, votes);
    if (errno == ENOMEM)
        return errno;
    if (type == T_GEN) {
        char * genre;
        for (genre = strtok(genres, DELIM_GENRE); genre != NULL; genre = strtok(NULL, DELIM_GENRE)) {
            addToGenre(imdb->first, year, genre);
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
        return NERR;
    int aux = imdb->iterY->year;
    imdb->iterY = imdb->iterY->tail;
    return aux;
}

void toBeginGenre(imdbADT imdb, int year){
    tLYear aux = searchYear( imdb->first , year);
    imdb->iterG = aux->first;
}

int hasNextGenre(imdbADT imdb){
    return imdb->iterG != NULL;
}

int nextGenre(imdbADT imdb){
    if(!hasNextGenre(imdb))
        return NERR;
    imdb->iterG = imdb->iterG->tail;
    return !NERR;
}

int getQ1(imdbADT imdb, char * buff) {
    int year = imdb->iterY->year;
    size_t films = imdb->iterY->cant[MOVIE];
    size_t series = imdb->iterY->cant[SERIES];
    return sprintf(buff, "%d%s%zu%s%zu", year, DELIM ,films, DELIM, series);
}

int getQ2(imdbADT imdb, char * buff, int year) {
    size_t films = imdb->iterG->cantFilms;
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
    // disculpas de antemano O.o
    return sprintf(buff, "%d%s%s%s%zu%s%.1f%s%s%s%zu%s%.1f", year, DELIM, film, DELIM, votesFilm, DELIM, ratingFilm, DELIM, serie, DELIM, votesSerie, DELIM, ratingSeries);
}