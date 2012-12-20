TARGET			= betweenness-preference
MAIN			= betweenness.cpp
MAIN_HEADER		= betweenness.h
GRAMMAR_FILE		= betweenness.lex
SCANNER 		= flex
SCANNER_FILE		= standard.yy.c
SCANNER_HEADER		= standard.yy.h
SCANNER_OPTIONS		= --outfile=${SCANNER_FILE} --header-file=${SCANNER_HEADER}
CPP 			= g++
CPPFLAGS 		= -g -Wall -Wextra -pedantic -Wshadow
LDFLAGS			= -ll
OBJS			= standard.yy.o parse.o
RM			= rm

all: ${TARGET}

${TARGET}: ${SCANNER_FILE} ${MAIN_HEADER} ${MAIN}
	${CPP} ${SCANNER_FILE} ${MAIN} ${CPPFLAGS} ${LDFLAGS} -o $@

${SCANNER_FILE}:  ${GRAMMAR_FILE}
	${SCANNER} ${SCANNER_OPTIONS} ${GRAMMAR_FILE}

clean:
	${RM} -f *.o ${TARGET} ${SCANNER_FILE} ${SCANNER_HEADER} *~
