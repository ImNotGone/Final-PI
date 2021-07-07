COMPILER 	= gcc
FLAGS 		= -pedantic -std=c99 -Wall
LINK_FLAGS 	= $(FLAGS) -fsanitize=address
DEBUG 		= -g
BINARY 		= imdb
OBJS 		= main.o imdbADT.o
QUERYS 		= query1.csv query2.csv query3.csv

# El fsanitize=address tira error cuando se 
# compilan los objetos asi que lo pongo aca
all: $(OBJS)
	$(COMPILER) $(LINK_FLAGS) $(OBJS) -o $(BINARY)

debug: FLAGS += $(DEBUG)
debug: all

main.o: main.c ADT-imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c main.c

imdbADT.o: ADT-imdb/imdbADT.c ADT-imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c ADT-imdb/imdbADT.c

clean: cleanQuerrys cleanBinary cleanObjs

cleanQuerrys:
	rm -rf $(QUERYS)

cleanBinary:
	rm -rf $(BINARY)

cleanObjs:
	rm -rf $(OBJS)
