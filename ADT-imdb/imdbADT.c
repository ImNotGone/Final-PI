#include "imdbADT.h"
#include <stdlib.h>

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
    size_t cantFilms;
    size_t cantSeries;
    tLGenre first;
    tLGenre iterG;
    tMediaInfo film;
    tMediaInfo series; 
    struct tNYear * tail; 
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;
    tLYear iterA;
} imdbCDT;


imdbADT newImdb() { 
    return calloc(1, sizeof(imdbCDT));
}