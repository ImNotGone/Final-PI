#include "imdbADT.h"
#include <stdlib.h>
#include <string.h>
#define BLOCK 10

typedef struct tMediaInfo {
    char * title;
    size_t dimTitle;
    size_t cantVotos;
    float rating;
} tMediaInfo;

typedef struct tNGenre {
    char * genre;
    size_t dimGenre;
    size_t cantFilms;
    struct tNGenre * tail;
} tNGenre;

typedef tNGenre * tLGenre;

typedef struct tNYear {
    int year;
    size_t cant[CANT_TYPES]; // La pelicula en la pos. 0 y la serie en la pos. 1 
    tLGenre first;
    tLGenre iterG;
    tMediaInfo media [CANT_TYPES]; // La pelicula en la pos. 0 y la serie en la pos. 1 
    struct tNYear * tail; 
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;
    tLYear iterY;
} imdbCDT;

static int compareYear(int year1, int year2) {
    return year1 - year2;
}

static int compareGenre(char * genre1, char * genre2) {
    return strcmp(genre1, genre2);
}

imdbADT newImdb() { 
    return calloc(1, sizeof(imdbCDT));
}

static char * copy (char * dest, char * source , size_t dim, size_t *newDim) {
    int i, j;
    for (i = dim, j = 0; source[j] != '\0'; j++) {
        if (j == 0 || i % BLOCK == 0){
            dest = realloc(dest, i + BLOCK + 1);
        }
        dest[i++] = source[j];
    }
    dest[i]='\0';
    dest = realloc(dest, i+1);
    *newDim = i;
    return dest;
}

static void addMedia( tMediaInfo * media , char * newTitle , float newRating , size_t newvotes) {
    media->cantVotos = newvotes;
    media->rating = newRating;
    media->title = copy(media->title, newTitle, media->dimTitle, &media->dimTitle);
}

static tLYear addToYear(tLYear first, titleType type, char * title, int year, float rating, size_t votes) {
    int c;
    if (first == NULL || (c=compareYear(year, first->year)) > 0 ) {
        tLYear aux = calloc(1, sizeof(tNYear));
        aux->year=year;
        aux->tail= first;
        aux->cant[type]++;
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
    if (first == NULL || (c=compareGenre(genre, first->genre)) > 0 ) {
        tLGenre aux = calloc(1, sizeof(tNGenre));
        aux->genre = copy( aux->genre, genre, 0, &aux->dimGenre);
        aux->cantFilms++;
        aux->tail = first;
        return first;
    }
    if ( c = 0) {
        first->cantFilms++;
        return first;
    }
    first->tail = addToGenreRec(first->tail, genre);
    return first;
}

static void addToGenre(tLYear first, int year, char * genre) {
    tLYear aux = first;
    while (aux != NULL && compareYear(aux->year, year) == 0) {
        aux = aux->tail;
    }
    aux->first = addToGenreRec(aux->first, genre);
}

void addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres) {
    imdb->first = addToYear(imdb, type, title, year, rating, votes);
    if (type == MOVIE) {
        char * genre;
        for (genre = strtok(genres, DELIM_GENRE); genre != NULL; genre = strtok(NULL, DELIM_GENRE)) {
             addToGenre(imdb->first, year, genre);   
        }
    }
}