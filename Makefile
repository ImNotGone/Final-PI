COMPILER 	= gcc
FLAGS 		= -pedantic -std=c99 -Wall
LINK_FLAGS 	= $(FLAGS) -fsanitize=address
DEBUG 		= -g
BINARY 		= imdb
OBJS 		= main.o imdbADT.o
QUERIES 	= query1.csv query2.csv query3.csv

# -fsanitize=address deberia usarse en la 
# linkedicion por lo que la dejo aca
all: $(OBJS)
	$(COMPILER) $(LINK_FLAGS) $(OBJS) -o $(BINARY)

debug: FLAGS += $(DEBUG)
debug: all

main.o: main.c ADT-imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c main.c

imdbADT.o: ADT-imdb/imdbADT.c ADT-imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c ADT-imdb/imdbADT.c

clean: cleanQueries cleanBinary cleanObjs

cleanQueries:
	rm -rf $(QUERIES)

cleanBinary:
	rm -rf $(BINARY)

cleanObjs:
	rm -rf $(OBJS)
