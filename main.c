#include "ADT-imdb/imdbADT.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Macros utiles
#define CANT_QUERYS 3
// Elegimos un valor arbitrarion para el tamanio del buffer acorde a lo que nos
// parecio necesario
#define BUFF_SIZE 512 
#define FALSE 0
#define TRUE !FALSE
#define NO_GENRE "Undefined"
#define DELIM ";"
#define DELIM_GENRE ","

// Headers para los archivos de las querys
#define HEADER1 "year;films;series"
#define HEADER2 "year;genre;films"
#define HEADER3 "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie"

// Las funciones loadQ*() (1, 2 y 3) devuelven ELOAD si tuvieron algun error
#define ELOAD (-1)

// Columnas del .csv recibido no ponemos runtime (que seria la columna 8) porque no se usa
// Al poner CANT_FIELDS en la ultima posicion, nos da la longitud del enum
typedef enum FIELDS {TYPE = 0, TITLE, S_YEAR, E_YEAR, GENRES, RATING, VOTES, CANT_FIELDS} FIELDS; 

// Aborta el programa con el valor que recibe
// enviando un mensaje a la salida de error
void errNOut(const char * errMsg, int exitCode);

// Cierra los archivos que recibe
void closeFiles(FILE ** files, size_t fileCount);

// Libera los recursos utilizados, aborta el programa con el codigo indicado
void closeNExit(imdbADT imdb, FILE ** files, size_t fileCount, const char * errMsg, int exitCode);



// Las funciones auxiliares loadQ*() (1, 2 y 3) reciben el FILE *
// donde se desea cargar con fprintf(), cada una "resuelve" una query 
// si hubo algun error en la carga de los datos, las funciones loadQ*() (1, 2 y 3) devuelven "ELOAD"
int loadQ1(imdbADT imdb, FILE * file, int year);

int loadQ2(imdbADT imdb, FILE * file, int year);

int loadQ3(imdbADT imdb, FILE * file, int year);

// Funcion auxiliar para liberar la memoria utilizada para la carga de datos en loadQ3()
// A su vez permite liberar la memoria en caso de que falle prematuramente el loadQ3() (ver comentarios de loadQ3())
void freeTitles(char ** titles, int dim);

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
    
    // Se verifica que fopen no tuviese errores en caso de que los hubiese se cierran los archivos
    // y se manda a salida de error un mensaje junto con errno como exit value.
    // Decidimos no chequear con errno, debido a la larga extencion de errores 
    // que evaluamos como "posibles" en nuestro programa
    // (EACCES, EROFS, EMFILE, ENFILE, ENOENT, ENAMETOOLONG, etc...).
    // Consideramos que era mejor verificar que ningun puntero a archivo sea NULL 
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
    if (imdb == NULL || errno == ENOMEM) {
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
        for(size_t pos = 0; pos < CANT_FIELDS && token != NULL && validYear; pos++, token = strtok(NULL, DELIM)) {
            // Tomamos en consideracion unicamente si no existe el anio para la carga de datos.
            // En caso de que tanto el rating como los votos sean "NONE" asumimos que valen 0 
            // (atoi y atof devuelven 0 en caso que no sean numeros). 
            // Si el tipo es distinto de "movie" o "tvSeries", lo ignoramos para el data entry.
            // No importa lo que sea el genero (incluso si es \N) lo tomamos en cosideracion como valido
            // pero enviamos algo mas descriptivo "NO_GENRE" 
            switch (pos) {
                case TYPE: type = token; break;
                case TITLE: title = token; break;
                case S_YEAR:
                    if (strcmp(token, NONE) == 0) {
                        // Si el anio era "NONE" nos lo saltemos en la carga de datos
                        // Ademas se corta el forloop (mas eficiente)
                        validYear = FALSE; 
                    } 
                    year = atoi(token);
                    break;
                case GENRES:
                    genres = token;
                    if (strcmp(token, NONE) == 0)
                        genres = NO_GENRE; // Si el genero era "NONE" enviamos "NO_GENRE"
                    break;
                case RATING: rating = atof(token); break;
                case VOTES: votes = atoi(token); break;
            }
        }
        // Si el anio es valido cargamos los datos acorde al tipo recibido
        if (validYear) {
            // Se usan estos condicionales para evitar el caso en el que haya mas de 2 categorias
            if (strcmp(type, "movie") == 0) {
                addToYear(imdb, MOVIE, title, year, rating, votes);
                // Si bien seria mas eficiente separar los generos en el back-end 
                // (para buscar una unica vez el nodo de la lista de anios por "grupo de generos")
                // decidimos que el front-end deberia encargarse del parseo de TODOS los datos
                // y el back-end unicamente de procesarlos
                for (char * genre = strtok(genres, DELIM_GENRE); genre != NULL; genre = strtok(NULL, DELIM_GENRE)) {
                    addToGenre(imdb, genre, year);
                }
            } else if (strcmp(type, "tvSeries") == 0) {
                addToYear(imdb, SERIES, title, year, rating, votes);
            }
        }
        // Verificamos que no hubiese errores de memoria,
        // si los hubiera notificamos al usuario mediante la salida de error
        // utilizamos "ENOMEM" como error de salida (OUT OF MEMORY en errno.h)
        if (errno == ENOMEM) {
            closeNExit(imdb, files, fileCount , "No hay memoria disponible en el heap", ENOMEM);
        }
    }

    /*================ HEADERS ================*/
    // Imprimimos usando fprintf los headers necesarios a cada archivo
    fprintf(query1, "%s\n", HEADER1);
    fprintf(query2, "%s\n", HEADER2);
    fprintf(query3, "%s\n", HEADER3);

    // Iniciamos el iterador
    toBeginYear(imdb);
    // Usamos while(hasNextYear), para saber cuando debemos dejar de iterar por anio 
    while(hasNextYear(imdb)) {
        // Ya que todas las querys usan el anio, llamo a getYear() una unica vez
        if ((year = getYear(imdb)) == EYEAR)
            closeNExit(imdb, files, fileCount, "Hubo un error al recibir informacion del TAD", EYEAR);
        
        /*============== QUERY 1 ==============*/
        // 1) 
        // chequeo que loadQ*() sea distinto de "ELOAD"
        if (loadQ1(imdb, query1, year) == ELOAD)
            closeNExit(imdb, files, fileCount, "Hubo un error al cargar los datos de la Q1", ELOAD);
        
        /*============== QUERY 3 ==============*/
        // Idem (1)
        if (loadQ3(imdb, query3, year) == ELOAD)
            closeNExit(imdb, files, fileCount, "Hubo un error al cargar los datos de la Q3", ELOAD);

        // Iniciamos el iterador de genero con el anio actual
        toBeginGenre(imdb, year);
        // Usamos while(hasNextGenre), para saber cuando debemos dejar de iterar por genero
        while(hasNextGenre(imdb)) {
            /*============== QUERY 2 ==============*/
            // Idem (1)
            if (loadQ2(imdb, query2, year) == ELOAD)
                closeNExit(imdb, files, fileCount, "Hubo un error al cargar los datos de la Q2", ELOAD);

            // Avanzamos la posicion del iterador por genero
            // 2)
            // No deberia tener errores, pero verificamos por las dudas
            // si hubo algun error notificamos al usuario mediante la salida de error
            // utilizamos el exitCode "ENEXT"
            if (nextGenre(imdb) == ENEXT) 
                closeNExit(imdb, files, fileCount, "Hubo un error al iterar por genero", ENEXT);
        }
        // Avanzamos la posicion del iterador por anio
        // Idem (2)
        if (nextYear(imdb) == ENEXT) 
            closeNExit(imdb, files, fileCount, "Hubo un error al iterar por anios", ENEXT);
    }
    /*============== CERRADO DE ARCHIVOS ==============*/
    closeFiles(files, fileCount);
    /*============= LIBERACION DE MEMORIA =============*/
    freeImdb(imdb);
    return 0;
}

int loadQ1(imdbADT imdb, FILE * file, int year) {
    size_t cantYear[CANT_TYPES];
    // En caso de tener un error al usar getCant() retorno lo antes posible
    // para notificar al usuario
    for (int i = 0; i < CANT_TYPES; i++) {
        if ((cantYear[i] = getCant(imdb, i)) == ECANT)
            return ELOAD;
    }
    int ret = fprintf(file, "%d%s%zu%s%zu\n", year, DELIM, cantYear[MOVIE], DELIM, cantYear[SERIES]);
    // 3)
    // Uso la variable ret por claridad en el if
    // Si fprintf() me devolvio negativo hubo algun error
    if (ret < 0) {
        return ELOAD;
    }
    return OK;
}

int loadQ2(imdbADT imdb, FILE * file, int year) {
    size_t cant;
    char * genre;
    // Hacemos los if's por separado por claridad unicamente, se podrian poner en una cadena de or's
    if ((cant = getCantGenre(imdb)) == ECANT)
        return ELOAD;
    if ((genre = getGenre(imdb)) == NULL || errno == ENOMEM) 
        return ELOAD;
    int ret = fprintf(file, "%d%s%s%s%zu\n", year, DELIM, genre, DELIM, cant);
    free(genre); // Libero la memoria reservada por getGenre()
    // Idem (3)
    if (ret < 0) {
        return ELOAD;
    }
    return OK;
}

int loadQ3(imdbADT imdb, FILE * file, int year) {
    char * titleMax[CANT_TYPES];
    size_t votesMax[CANT_TYPES];
    float ratingMax[CANT_TYPES];
    int i;
    for (i = 0; i < CANT_TYPES; i++) {
        // Hacemos los if's por separado por claridad unicamente, se podrian poner en una cadena de or's
        if ((titleMax[i] = getTitle(imdb, i)) == NULL || errno == ENOMEM) {
            // 4)
            // Liberamos la memoria reservada previo al fallo
            // por si por ejemplo falla en la segunda pasada del ciclo
            freeTitles(titleMax, i); 
            return ELOAD;
        }
        if ((votesMax[i] = getVotes(imdb, i)) == ECANT) {
            // Idem (4)
            freeTitles(titleMax, i);
            return ELOAD;
        }
        if ((ratingMax[i] = getRating(imdb, i)) == ECANT) {
            // Idem (4)
            freeTitles(titleMax, i);
            return ELOAD;
        }
    }
    int ret = fprintf(file, "%d%s%s%s%zu%s%.1f%s%s%s%zu%s%.1f\n", year, DELIM, titleMax[MOVIE], DELIM, votesMax[MOVIE], DELIM, ratingMax[MOVIE], DELIM, titleMax[SERIES], DELIM, votesMax[SERIES], DELIM, ratingMax[SERIES]);
    freeTitles(titleMax, i); // Liberamos los titulos
    // Idem (3)
    if (ret < 0) {
        return ELOAD;
    }
    return OK;
}

void freeTitles(char ** titles, int dim) {
    for (int i = 0; i < dim; i++) {
        free(titles[i]);
    }
}

void closeNExit(imdbADT imdb, FILE ** files, size_t fileCount, const char * errMsg, int exitCode) {
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
        if (files[i] != NULL) 
            fclose(files[i]);
}