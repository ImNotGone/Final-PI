#include "imdbADT.h"
#include <stdlib.h>
#include <string.h>

typedef struct tMediaInfo {
    char * title;
    size_t dimTitle;
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
    tMediaInfo media [CANT_TYPES]; // La pelicula en la pos. 0 y la serie en la pos. 1 
    struct tNYear * tail; 
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;
    tLYear iter;
} imdbCDT;

static int compareYear(int year1, int year2){
    return year1 - year2;
}

imdbADT newImdb() { 
    return calloc(1, sizeof(imdbCDT));
}

static char * copy (char * dest, char * source , size_t dim, size_t *newDim){
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
    if (first == NULL || (c=compareYear(year, first->year)) > 0 ){
        tLYear aux = calloc(1,sizeof(tNYear));
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
    first->tail = addToYear(first->tail, year);
    return first;
}



void addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genre) {
    imdb->first = addToYear(imdb, type, title, year, rating, votes);
    
    
}