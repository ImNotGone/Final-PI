#ifndef IMDB_ADT_H
#define IMDB_ADT_H

#include <stdlib.h>
#include <errno.h>

// Macros generales
#define NONE "\\N"
#define DELIM ";"
#define DELIM_GENRE ","

#define OK 1
// los "next" para iterar retornan ENEXT si hubo algun error
#define ENEXT (-1)

// CANT_TYPES debe ir siempre al final para que se defina correctamente la cantidad de tipos
typedef enum {MOVIE = 0, SERIES, CANT_TYPES} titleType; 
// Para determinar a que tipo se le trackean los generos
#define TRACK_GENRE_TO MOVIE 

typedef struct imdbCDT * imdbADT;

// Genera TAD vacio para almacenar los datos necesarios
// Si no se pudo crear retorna NULL
imdbADT newImdb(void);

// Libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

// Permite la carga de datos que recibe al TAD
// trakea unicamente la cantidad de Media por genero que se eligio en "T_GEN"
// si hubo algun error de memoria retorna "ENOMEM"
// sino retorna "OK" 
int addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres);

// Inicia el iterador por anios
void toBeginYear(imdbADT imdb);

// Verifica que haya un siguiente elemento para el iterador de anios
// retorna 1 si se puede seguir iterando 0 si no
int hasNextYear(imdbADT imdb);

// Avanza la posicion del iterador por anio
// si hubo algun error retorna "ENEXT"
// sino retorna el anio
int nextYear(imdbADT imdb);

// Inicia el iterador por generos en el anio indicado
void toBeginGenre(imdbADT imdb, int year);

// Verifica que haya un siguiente elemento para el iterador de generos en el anio indicado
// retorna 1 si se puede seguir iterando 0 si no
int hasNextGenre(imdbADT imdb);

// Avanza la posicion del iterador por genero
// si hubo algun error retorna "ENEXT"
// sino retorna "OK"
int nextGenre(imdbADT imdb);

// Carga la informacion necesaria para la query 1 en el buffer
// en caso de que el buffer no sea de tamanio suficiente retorna (-1)
// sino retornan la longitud de el string que se cargo en el buffer
int getQ1(imdbADT imdb, char * buff);

// Carga la informacion necesaria para la query 2 en el buffer
// en caso de que el buffer no sea de tamanio suficiente retorna (-1)
// sino retornan la longitud de el string que se cargo en el buffer
int getQ2(imdbADT imdb, char * buff, int year);

// Carga la informacion necesaria para la query 3 en el buffer
// en caso de que el buffer no sea de tamanio suficiente retorna (-1)
// sino retornan la longitud de el string que se cargo en el buffer
int getQ3(imdbADT imdb, char * buff);

#endif //IMDB_ADT_H
