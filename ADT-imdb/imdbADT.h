#ifndef IMDB_ADT_H
#define IMDB_ADT_H

typedef struct imdbCDT * imdbADT;

typedef enum titleType{MOVIE = 0, SERIES};

// Genera TAD vacio para almacenar los datos necesarios
// Si no se pudo crear retorna NULL
imdbADT newImdb(void);

// Libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

// Permite la carga de datos al TAD
// recibe una linea con la informacion a cargar
void addData(imdbADT imdb, char * data);

#endif //IMDB_ADT_H
