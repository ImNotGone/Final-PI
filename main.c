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
#define NO_GENRE "Undefined"
#define HEADER1 "year;films;series"
#define HEADER2 "year;genre;films"
#define HEADER3 "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie"

// las funciones getQ* (1, 2 y 3) retornan -1 en caso de que el buffer fuese muy chico
#define BUFF_OF (-1) 

// Columnas del .csv recibido no ponemos runtime (que seria la columna 8) porque no se usa
// Al poner CANT_DIVIDERS en la ultima posicion, nos da la longitud del enum (=7)
typedef enum DIVIDERS {TYPE = 0, TITLE, S_YEAR, E_YEAR, GENRES, RATING, VOTES, CANT_DIVIDERS} DIVIDERS; 

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
    // Si la cantidad de parametros recibida no es 2 (el programa[0] y el path[1])
    // terminamos el programa y notificamos al usuario mediante la salida de error
    // utilizamos "EINVAL" como error de salida (INVALID ARGUMENT en errno.h)
    if (cantArg != 2)
        errNOut("Cantidad invalida de parametros", EINVAL);

    /*========= INICIO DE VARIABLES =========*/
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
    // errno como exit value.
    // Decidimos no chequear con errno, debido a la larga extencion de errores
    // que considerabamos "posibles" en nuestro programa
    // (EACCES, EROFS, EMFILE, ENFILE, ENOENT, EROFS, ENAMETOOLONG, etc...)
    // que puede llegar a enviar fopen, por lo que consideramos que era mejor
    // verificar que ningun puntero a archivo sea NULL 
    // y enviar el errno en conjunto con la salida
    for(int i = 0; i < fileCount; i++) {
        if (files[i] == NULL) {
            closeFiles(files, fileCount);
            errNOut("Hubo un error al abrir un archivo", errno);
        }
    }

    // Inicio el TAD para la carga de datos
    imdbADT imdb = newImdb();
    // Si el TAD no se pudo crear notifico al usuario mediante la salida de error
    // utilizamos "ENOMEM" como error de salida (OUT OF MEMORY en errno.h)
    if (imdb == NULL || errno == ENOMEM){
        closeFiles(files, fileCount);
        errNOut("No hay memoria disponible en el heap", ENOMEM);
    }

    char buff[BUFF_SIZE], * type, * title, * genres, * token;
    int year, votes, validYear; // validYear para verificar que el anio sea valido
    float rating;
    
    // Levantamos la primera linea que no contiene informacion
    // A pesar de que se asume que los datos son correctos, verificamos que el
    // archivo no este vacio. 
    // Si estaba vacio notificamos al usuario mediante salida de error
    // utilizamos "EINVAL" como error de salida (INVALID ARGUMENT en errno.h)
    if (fgets(buff, BUFF_SIZE, data) == NULL) {
        closeNExit(imdb, files, fileCount, "El archivo recibido estaba vacio", EINVAL);
    }

    /*================ CARGA DE DATOS AL TAD ================*/
    while(fgets(buff, BUFF_SIZE, data) != NULL) {
        validYear = TRUE;
        token = strtok(buff, DELIM);
        for(size_t pos = 0; pos < CANT_DIVIDERS && token != NULL; pos++, token = strtok(NULL, DELIM)) {
            // Tomamos en consideracion unicamente si no existe el anio para la carga de datos.
            // En caso de que tanto el rating como los votos sean NONE o \N asumimos que valen 0 
            // (atoi y atof devuelven 0 en caso que no sean numeros). 
            // Si el tipo es distinto de "movie" o "tvSeries", lo ignoramos para el data entry.
            // No importa lo que sea el genero (incluso si es \N) lo tomamos en cosideracion como valido
            // pero enviamos algo mas descriptivo "NO_GENRE" (="Undefined")
            switch (pos) {
                case TYPE: type = token; break;
                case TITLE: title = token; break;
                case S_YEAR:
                    if (strcmp(token, NONE) == 0)
                        validYear = FALSE; // si el anio era NONE nos lo saltemos en la carga de datos
                    year = atoi(token);
                    break;
                case GENRES:
                    genres = token;
                    if (strcmp(token, NONE) == 0)
                        genres = NO_GENRE; // si el genero era none enviamos NO_GENRE (="Undefined")
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
        // verificamos que no hubiese errores de memoria,
        // si los habia notificamos al usuario mediante la salida de error
        // utilizamos "ENOMEM" como error de salida (OUT OF MEMORY en errno.h)
        if(errno == ENOMEM) {
            closeNExit(imdb, files, fileCount , "No hay memoria disponible en el heap", ENOMEM);
        }
    }

    /*================ HEADERS ================*/
    // Imprimimos usando fprintf los headers necesarios a cada archivo
    fprintf(query1, "%s\n", HEADER1);
    fprintf(query2, "%s\n", HEADER2);
    fprintf(query3, "%s\n", HEADER3);
    
    // 1) 
    // Si hay algun error debido al tamanio del buffer utilizado
    // getQ* (1, 2 y 3) devuelven BUFF_OF, este valor de retorno se utiliza
    // para poder notificar al usuario mediante la salida de error

    // Iniciamos el iterador
    toBeginYear(imdb);
    // Usamos while(hasNext), para saber cuando debemos dejar de iterar 
    while(hasNextYear(imdb)) {
        /*============== QUERY 1 ==============*/
        // Obtenemos la informacion necesaria para las lineas de la Q1
        // las enviamos al archivo mediante fprintf
        if (getQ1(imdb, buff) == BUFF_OF) {
            closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
        }
        fprintf(query1, "%s\n", buff);
        /*============== QUERY 3 ==============*/
        // Obtenemos la informacion necesaria para las lineas de la Q3
        // las enviamos al archivo mediante fprintf
        if (getQ3(imdb, buff) == BUFF_OF) {
            closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
        }
        fprintf(query3, "%s\n", buff);
        // obtenemos el anio actual y avanzamos la posicion del iterador por anio
        year = nextYear(imdb);
        // iniciamos el iterador de genero con el anio actual
        toBeginGenre(imdb, year);
        // Usamos while(hasNext), para saber cuando debemos dejar de iterar
        while(hasNextGenre(imdb)) {
            /*============== QUERY 2 ==============*/
            // Obtenemos la informacion necesaria para las lineas de la Q2
            // las enviamos al archivo mediante fprintf
            if (getQ2(imdb, buff, year) == BUFF_OF) {
                closeNExit(imdb, files, fileCount, "Se debe incrementar el tamanio del buffer", BUFF_OF);
            }
            fprintf(query2, "%s\n", buff);
            // avanzamos la posicion del iterador por genero
            nextGenre(imdb);
        }
        
    }
    /*============== CERRADO DE ARCHIVOS ==============*/
    closeFiles(files, fileCount);
    /*============= LIBERACION DE MEMORIA =============*/
    freeImdb(imdb);
    return 0;
}

void closeNExit(imdbADT imdb, FILE ** files, size_t fileCount, char * errMsg, int exitCode){
    freeImdb(imdb);
    closeFiles(files, fileCount);
    errNOut(errMsg, exitCode);
}

void errNOut(const char * errMsg, int exitCode) {
    fprintf(stderr, "%s\nExitCode: %d", errMsg, exitCode);
    exit(exitCode);
}

void closeFiles(FILE * files[], size_t fileCount) {
    for(size_t i = 0; i < fileCount; i++)
        if(files[i] != NULL) 
            fclose(files[i]);
}