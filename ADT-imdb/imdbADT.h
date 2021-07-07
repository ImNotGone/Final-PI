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

// Para comenzar el iterador de anios
void toBeginForYears(imdbADT imdb);

// Retorna 1 si hay un anio siguiente , 0 si llego al ultimo
int hasNextForYears(imdbADT imdb);

// Pasa al siguiente anio si es que existe, luego retorna en su nombre el ultimo 
// anio en el que estuvo parado para asi mostrar cantidad de series y peliculas
tLYear NextForYears(imdbADT imdb);

// Para comenzar el iterador por generos
void toBeginForGenre(imdbADT imdb);

//retorna 1 si hay otro genero , 0 si llego al ultimo
int hasNextForGenre(imdbADT imdb);

// Pasa al siguiente genero si es que existe, luego retorna en su nombre el
// ultimo genero al que accedio para luego mostrar cuantas peliculas hubieron
tLGenre nextForGenre(imdbADT imdb);

#endif //IMDB_ADT_H
