TARGET			= bats
MAIN			= parse.cpp
CLASSES			= classes.cpp
HEADERS			= classes.h
GLOBALDEFS		= global_defs.cpp
GRAMMAR_FILE		= scanner.lex
SCANNER 		= flex
SCANNER_FILE		= standard.yy.c
SCANNER_HEADER		= standard.yy.h
SCANNER_OPTIONS		= --outfile=${SCANNER_FILE} --header-file=${SCANNER_HEADER}
CPP 			= g++
DEBUG			= -g
OPTIM			= -O3
CPPFLAGS 		= -Wall -Wextra -pedantic -Wshadow ${DEBUG}
LDFLAGS			= -lfl -lboost_date_time -ligraph -lsqlite3 -lboost_program_options
OBJS			= classes.o
RM			= rm

all: ${TARGET}

${TARGET}: ${SCANNER_FILE} ${OBJS} ${GLOBALDEFS} ${MAIN}
	${CPP} ${SCANNER_FILE} ${OBJS} ${MAIN} ${CPPFLAGS} ${LDFLAGS} -o $@

${OBJS}:  ${HEADERS} ${CLASSES}
	${CPP} -c ${CLASSES}

${SCANNER_FILE}:  ${GRAMMAR_FILE}
	${SCANNER} ${SCANNER_OPTIONS} ${GRAMMAR_FILE}

clean:
	${RM} -f *.o ${TARGET} ${SCANNER_FILE} ${SCANNER_HEADER} *~
