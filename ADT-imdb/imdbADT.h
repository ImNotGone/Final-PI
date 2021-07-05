#ifndef IMDB_ADT_H
#define IMDB_ADT_H

typedef struct imdbCDT * imdbADT;

//devuelve un nuevo TAD vacio
imdbADT newImdb(imdbADT imdb);

//libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

//agrega la informacion correspondiente al TAD
void addData(imdbADT imdb, char * data);

#endif //IMDB_ADT_H
