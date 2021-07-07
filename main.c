#include "ADT-imdb/imdbADT.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define CANT_QUERYS 3
#define BUFF_SIZE 512 
#define FALSE 0
#define TRUE !FALSE
#define HEADER1 "year;films;series"
#define HEADER2 "year;genre;films"
#define HEADER3 "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie"

typedef enum ERRORS {ARGC = 1, INV_FILE, NO_MEM} ERRORS;

#define CANT_DIVIDERS 7
// no ponemos runtime porque no se usa
typedef enum DIVIDERS {TYPE = 0, TITLE, S_YEAR, E_YEAR, GENRES, RATING, VOTES} DIVIDERS; 

// Aborta el programa con el valor que recibe
// enviando un mensaje a la salida de error
void errNOut(const char * errMsg, int exitCode);

// Cierra los archivos que recibe
void closeFiles(FILE ** files, size_t fileCount);

// Libera los recursos utilizados, aborta el programa 
// con el codigo indicado
void closeNExit(imdbADT imdb, FILE ** files, size_t fileCount, char * errMsg,int exitCode);

// Permite el procesamiento de los datos de Imdb
int main(int cantArg, char * args[]) {
    if (cantArg != 2)
        errNOut("Cantidad invalida de parametros", ARGC);

    errno = 0;
    FILE * data = fopen(args[1], "r");
    FILE * query1 = fopen("query1.csv", "w+");
    FILE * query2 = fopen("query2.csv", "w+");
    FILE * query3 = fopen("query3.csv", "w+");
    FILE * files[] = {data, query1, query2, query3};
    size_t fileCount = CANT_QUERYS + cantArg - 1;
    // Se verifica que fopen no tuviese errores
    // en caso de que los hubiese se cierran los archivos
    // y se manda a salida de error un mensaje junto con 
    // errno como exit value
    for(int i = 0; i < fileCount; i++) {
        if(files[i] == NULL) {
            closeFiles(files, fileCount);
            errNOut("Hubo un error al abrir un archivo", errno);
        }
    }

    imdbADT imdb = newImdb();
    if (imdb == NULL){
        closeFiles(files, fileCount);
        errNOut("No hay memoria disponible en el heap", NO_MEM);
    }

    /*================ CARGA DE DATOS ================*/
    char buff[BUFF_SIZE], * type, * title, * genres, * token;
    int year, votes, validYear; // validYear para verificar que el anio sea valido
    float rating;
    
    // Levantamos la primera linea que no contiene informacion
    // No verificamos que el archivo este vacio  
    // puesto que se asume que los datos son correctos
    fgets(buff, BUFF_SIZE, data);

    while(fgets(buff, BUFF_SIZE, data) != NULL) {
        validYear = TRUE;
        token = strtok(buff, DELIM);
        for(size_t pos = 0; pos < CANT_DIVIDERS && token != NULL; pos++, token = strtok(NULL, DELIM)) {
            // Se asumio que no hay ningun NONE en {type, title, year, genre, rating, votes}
            // Ya que en el DOC de la consigna se asume que los datos son correctos
            switch (pos) {
                case TYPE: type = token; break;
                case TITLE: title = token; break;
                case S_YEAR:
                    if ( strcmp(token, NONE) == 0)
                        validYear = FALSE;
                    year = atoi(token);
                    break;
                case GENRES: genres = token; break;
                case RATING: rating = atof(token); break;
                case VOTES: votes = atoi(token); break;
            }
        }
        if (validYear) {
            if (strcmp(type, "movie") == 0) {
                if (addData(imdb, MOVIE, title, year, rating, votes, genres) == MEM_ERROR) {
                    /* COSAS DE ERROR */
                }
            } else if (strcmp(type, "tvSeries") == 0) {
                if (addData(imdb, SERIES, title, year, rating, votes, genres) == MEM_ERROR) {
                    /* COSAS DE ERROR */
                }
            }
        }
    }

    /*================ HEADERS ================*/
    fprintf(query1, "%s\n", HEADER1);
    fprintf(query2, "%s\n", HEADER2);
    fprintf(query3, "%s\n", HEADER3);
    
    toBeginYear(imdb);
    while(hasNextYear(imdb)) {
        /*============== QUERY 1 ==============*/
        if (getQ1(imdb, buff) == BUFF_OF) {
            closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
        }
        fprintf(query1, "%s\n", buff);
        /*============== QUERY 3 ==============*/
        if (getQ3(imdb, buff) == BUFF_OF) {
            closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
        }
        fprintf(query3, "%s\n", buff);
        year = nextYear(imdb);
        toBeginGenre(imdb, year);
        while(hasNextGenre(imdb)) {
            /*============== QUERY 2 ==============*/
            if (getQ2(imdb, buff, year) == BUFF_OF) {
                closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
            }
            fprintf(query2, "%s\n", buff);
            nextGenre(imdb);
        }
        
    }
    closeFiles(files, fileCount);
    freeImdb(imdb);
    return 0;
}

void closeNExit(imdbADT imdb, FILE ** files, size_t fileCount, char * errMsg, int exitCode){
    freeImdb(imdb);
    closeFiles(files, fileCount);
    errNOut(errMsg, exitCode);
}

void errNOut(const char * errMsg, int exitCode) {
    fprintf(stderr, "%s\n", errMsg);
    exit(exitCode);
}

void closeFiles(FILE * files[], size_t fileCount) {
    for(size_t i = 0; i < fileCount; i++)
        if(files[i] != NULL) 
            fclose(files[i]);
}