#ifndef IMDB_ADT_H
#define IMDB_ADT_H

#include <stdlib.h>
#include <errno.h>

#define BLOCK 10
#define DELIM ";"
#define DELIM_GENRE ","
#define NONE "\\N"
typedef struct imdbCDT * imdbADT;
#define CANT_TYPES 2
typedef enum {MOVIE = 0, SERIES} titleType;
#define T_GEN MOVIE
#define OK 1
#define BUFF_OF (-1) // sprintf retorna -1 si no entro el string en el buffer
#define NERR (-3)

// Genera TAD vacio para almacenar los datos necesarios
// Si no se pudo crear retorna NULL
imdbADT newImdb(void);

// Libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

// Permite la carga de datos al TAD
// recibe los datos a cargar en el TAD
int addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres);

// Inicia el iterador por anios
void toBeginYear(imdbADT imdb);

// Verifica que haya un siguiente elemento para el iterador de anios
int hasNextYear(imdbADT imdb);

// Avanza la posicion del iterador por anio
int nextYear(imdbADT imdb);

// Inicia el iterador por generos en el anio indicado
void toBeginGenre(imdbADT imdb, int year);

// Verifica que haya un siguiente elemento para el iterador de generos en el anio indicado
int hasNextGenre(imdbADT imdb);

// Avanza la posicion del iterador por genero
int nextGenre(imdbADT imdb);

// Carga la informacion necesaria para la query 1 en el buffer
int getQ1(imdbADT imdb, char * buff);

// Carga la informacion necesaria para la query 2 en el buffer
int getQ2(imdbADT imdb, char * buff, int year);

// Carga la informacion necesaria para la query 3 en el buffer
int getQ3(imdbADT imdb, char * buff);

#endif //IMDB_ADT_H
