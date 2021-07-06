#ifndef IMDB_ADT_H
#define IMDB_ADT_H

#define DELIM_GENRE ","
typedef struct imdbCDT * imdbADT;
#define CANT_TYPES 2
typedef enum{MOVIE = 0, SERIES} titleType;

// Genera TAD vacio para almacenar los datos necesarios
// Si no se pudo crear retorna NULL
imdbADT newImdb(void);

// Libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

// Permite la carga de datos al TAD
// recibe los datos a cargar en el TAD
void addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres);

#endif //IMDB_ADT_H
