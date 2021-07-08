#include "ADT-imdb/imdbADT.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define CANT_QUERYS 3
#define BUFF_SIZE 512 
#define FALSE 0
#define TRUE !FALSE
#define NO_GENRE "Undefined"
#define HEADER1 "year;films;series"
#define HEADER2 "year;genre;films"
#define HEADER3 "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie"

typedef enum ERRORS {ARGC = 1, INV_FILE} ERRORS;

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

#ifdef MEMCHK
#include <sys/resource.h>
static void
setMemLimit(size_t memLimit) {
    struct rlimit limit;
    limit.rlim_cur = memLimit;
    limit.rlim_max = memLimit;
    if (setrlimit(RLIMIT_AS, &limit) != 0) {
        fprintf(stderr, "setrlimit() failed with errno=%d\n", errno);
        exit(EXIT_FAILURE);
    }
    if (setrlimit(RLIMIT_DATA, &limit) != 0) {
        fprintf(stderr, "setrlimit() failed with errno=%d\n", errno);
        exit(EXIT_FAILURE);
    }
}
#endif


// Permite el procesamiento de los datos de Imdb
int main(int cantArg, char * args[]) {
    clock_t start = clock();

    if (cantArg != 2)
        errNOut("Cantidad invalida de parametros", ARGC);

    #ifdef MEMCHK
    setMemLimit(200 * sizeof(int)); // Limite de 5000 ints

    void * aux;
    long n = 0;
    do {
        errno = 0;
        aux = malloc(1024);
        n++;
    } while ( aux != NULL && !errno);
    printf("Falla despues de %ld KB   %ld MB\n", n, n / 1024 );
    #endif

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
            errNOut("Hubo un error al abrir un archivo", ENOMEM);
        }
    }

    imdbADT imdb = newImdb();
    if (imdb == NULL || errno == ENOMEM){
        closeFiles(files, fileCount);
        errNOut("No hay memoria disponible en el heap", ENOMEM);
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
            // Tomamos en consideracion unicamente si no existe el anio para la carga de datos.
            // En caso de que tanto el rating como los votos sean NONE o \N asumimos que valen 0 
            // (atoi y atof devuelven 0 en caso que no sean numeros). 
            // Si el tipo es distinto de "movie" o "tvSeries", lo ignoramos para el data entry.
            // No importa lo que sea el genero (incluso si es \N) lo tomamos en cosideracion como valido
            switch (pos) {
                case TYPE: type = token; break;
                case TITLE: title = token; break;
                case S_YEAR:
                    if ( strcmp(token, NONE) == 0)
                        validYear = FALSE;
                    year = atoi(token);
                    break;
                case GENRES:
                    genres = token;
                    if (strcmp(token, NONE) == 0)
                        genres = NO_GENRE;
                    break;
                case RATING: rating = atof(token); break;
                case VOTES: votes = atoi(token); break;
            }
        }
        if (validYear) {
            if (strcmp(type, "movie") == 0) {
                addData(imdb, MOVIE, title, year, rating, votes, genres);
            } else if (strcmp(type, "tvSeries") == 0) {
                addData(imdb, SERIES, title, year, rating, votes, genres);
            }
        }
        if(errno == ENOMEM) {
            closeNExit(imdb, files, fileCount , "No hay memoria disponible en el heap", ENOMEM);
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
    clock_t stop = clock();
    double elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
    printf("Execution successfull in: %.5f\n", elapsed);
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