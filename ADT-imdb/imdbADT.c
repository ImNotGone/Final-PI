#include "imdbADT.h"

typedef struct tMediaInfo {
    char * title;
    int cantVotos;
    float rating;
} tMediaInfo;

typedef struct tNGenre {
    char * genre;
    int cantFilms;
    struct tNGenre * tail;
} tNGenre;

typedef tNGenre * tLGenre;

typedef struct tNYear {
    int year;
    int cantFilms;
    int cantSeries;
    tLGenre first;
    tMediaInfo film;
    tMediaInfo series; 
    struct tNYear * tail; 
} tNYear;

typedef tNYear * tLYear;

typedef struct imdbCDT {
    tLYear first;
    tLYear iter;
} imdbCDT;
