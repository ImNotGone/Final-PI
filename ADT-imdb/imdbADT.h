#ifndef IMDB_ADT_H
#define IMDB_ADT_H

#include <stdlib.h>
#include <errno.h>

// Macro auxiliar para indicar un campo vacio
#define NONE "\\N"

// Return values
#define OK 1
#define ENEXT (-2)
#define EYEAR (-3)
#define ECANT (-4)

// CANT_TYPES debe ir siempre al final para que se defina correctamente la cantidad de tipos
typedef enum titleType {MOVIE = 0, SERIES, CANT_TYPES} titleType; 
// Para determinar a que tipo se le trackean los generos
#define TRACK_GENRE_TO MOVIE 

typedef struct imdbCDT * imdbADT;

// Genera TAD vacio para almacenar los datos necesarios
// Si no se pudo crear retorna NULL
imdbADT newImdb(void);

// Libera la memoria reservada por el TAD
void freeImdb(imdbADT imdb);

// Permite la carga de datos que recibe al TAD
// trakea unicamente la cantidad de Media por genero que se eligio en "TRACK_GENRE_TO"
// si hubo algun error de memoria retorna "ENOMEM" y setea errno en "ENOMEM"
// si el type es invalido retorna "!OK"
// sino retorna "OK" 
int addData(imdbADT imdb, titleType type, char * title, int year, float rating, size_t votes, char * genres, char * delimGenre);

// Inicia el iterador por anios
void toBeginYear(imdbADT imdb);

// Verifica que haya un siguiente elemento para el iterador de anios
// retorna 1 si se puede seguir iterando 0 si no
int hasNextYear(imdbADT imdb);

// Avanza la posicion del iterador por anio
// si hubo algun error retorna "ENEXT"
// sino retorna "OK"
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

// Devuelve el anio en el cual esta parado el iterador actualmente
// retorna "EYEAR" si hubo algun error al conseguir el anio
int getYear(imdbADT imdb);

// Devuelve la cantidad de veces que un "tipo" (MOVIE o SERIES)
// aparecio en el anio donde esta parado el iterador
// retorna "ECANT" si hubo algun error
size_t getCant(imdbADT imdb, titleType type);

// Devuelve un puntero a la zona de memoria que contiene una copia
// del nombre del "type" mas votado para el anio en el cual esta parado el iterador
// si hubo algun error retorna NULL y setea errno en "ENOMEM"
char * getTitle(imdbADT imdb, titleType type);

// Devuelve la cantidad de votos del "type" mas votado para 
// el anio en el cual esta parado el iterador
// retorna "ECANT" si hubo algun error
size_t getVotes(imdbADT imdb, titleType type);

// Devuelve el rating del "type" mas votado para 
// el anio en el cual esta parado el iterador
// retorna "ECANT" si hubo algun error
float getRating(imdbADT imdb, titleType type);

// Devuelve un puntero a una zona de memoria que contiene una copia
// del nombre del genero en el cual esta parado el iterador
// si hubo algun error retorna NULL y setea errno en "ENOMEM"
char * getGenre(imdbADT imdb);

// Devuelve la cantidad de veces que se uso el genero en el cual esta parado el iterador
// para describir el tipo trackeado segun la macro "TRACK_GENRE_TO"
// retorna "ECANT" si hubo algun error
size_t getCantGenre(imdbADT imdb);

#endif //IMDB_ADT_H
