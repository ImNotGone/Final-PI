#include "ADT-imdb/imdbADT.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define CANT_QUERYS 3
#define BUFF_SIZE 128 
#define DELIM ";"
#define NONE "/N"
#define CANT_DIVIDERS 8

typedef enum errors {ARGC = 1, INV_FILE, NO_MEM};
typedef enum info {TYPE = 0, TITLE, S_YEAR, E_YEAR, GENRES, RATING, VOTES, RUNTIME};

// Aborta el programa con el valor que recibe
// enviando un mensaje a la salida de error
void errNout(const char * errMsg, int exitCode);

// Cierra los archivos que recibe
void closeFiles(FILE ** files, size_t fileCount);

// Permite el procesamiento de los datos de Imdb
int main(int cantArg, char * args[]) {
    if (cantArg != 2)
        errNout("Cantidad invalida de parametros", ARGC);

    errno = 0;
    FILE * data = fopen(args[1], "r");
    FILE * query1 = fopen("query1.csv", "w+");
    FILE * query2 = fopen("query2.csv", "w+");
    FILE * query3 = fopen("query3.csv", "w+");
    FILE * files[] = {data, query1, query2, query3};
    size_t fileCount = CANT_QUERYS + cantArg - 1;
    if (errno == ENOENT) {
        closeFiles(files, fileCount);
        errNout("Hubo un error al abrir un archivo", INV_FILE);
    }

    // todo usar los archivos
    char buff[BUFF_SIZE], * type, * title, * genres, * token;
    int year, votes;
    float rating;
    imdbADT imdb = newImdb();
    if (imdb == NULL){
        closeFiles(files, fileCount);
        errNout("No hay memoria disponible en el heap", NO_MEM);
    }

    // Levantamos la primera linea que no contiene informacion
    // No verificamos que el archivo este vacio  
    // puesto que se asume que los datos son correctos
    fgets(buff, BUFF_SIZE, data);
    /*================ CARGA DE DATOS ================*/
    while(fgets(buff, BUFF_SIZE, data) != NULL) {
        token = strtok(buff, DELIM);
        for(size_t pos = 0; pos < CANT_DIVIDERS && token != NULL; pos++, token = strtok(NULL, DELIM)) {
            switch (pos) {
                case TYPE: type = token; break;
                case TITLE: title = token; break;
                case S_YEAR: year = atoi(token); break;
                case GENRES: genres = token; break;
                case RATING: rating = atof(token); break;
                case VOTES: votes = atoi(token); break;
            }
        }
        if (strcmp(type, "movie") == 0) {
            addData(imdb, MOVIE, title, year, rating, votes, genres);
        } else if (strcmp(type, "tvSeries") == 0) {
            addData(imdb, SERIES, title, year, rating, votes, NULL);
        }
    }

    /*================ QUERY 1 ================*/
    /*code*/
    
    /*================ QUERY 2 ================*/
    /*code*/

    /*================ QUERY 3 ================*/
    /*code*/
    closeFiles(files, fileCount);
    return 0;
}

void errNout(const char * errMsg, int exitCode) {
    fprintf(stderr, "%s\n", errMsg);
    exit(exitCode);
}

void closeFiles(FILE * files[], size_t fileCount) {
    for(size_t i = 0; i < fileCount; i++)
        if(files[i] != NULL) 
            fclose(files[i]);
    return;
}
