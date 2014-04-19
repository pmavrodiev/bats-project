TARGET			= bats
MAIN			= parse.cpp
CLASSES			= classes.cpp
HEADERS			= classes.h
GLOBALDEFS		= global_defs.cpp
GRAMMAR_CONFIG_FILE	= scanner_config.lex
GRAMMAR_DATA_FILE	= scanner_data.lex
SCANNER 		= flex
SCANNER_CONFIG_FILE	= config_file.yy.c
SCANNER_CONFIG_HEADER	= config_file.yy.h
SCANNER_CONFIG_OPTIONS	= --outfile=${SCANNER_CONFIG_FILE} --header-file=${SCANNER_CONFIG_HEADER} --prefix="config_"
SCANNER_DATA_FILE	= data_file.yy.c
SCANNER_DATA_HEADER	= data_file.yy.h
SCANNER_DATA_OPTIONS	= --outfile=${SCANNER_DATA_FILE} --header-file=${SCANNER_DATA_HEADER} --prefix="data_"
CPP 			= g++
DEBUG			= -g
OPTIM			= -O3
CPPFLAGS 		= -Wall -Wextra -pedantic -Wshadow ${DEBUG}
LDFLAGS			= -lfl -lboost_date_time -ligraph -lz -lsqlite3 -lboost_program_options
OBJS			= classes.o
RM			= rm

all: ${TARGET}

${TARGET}: ${SCANNER_CONFIG_FILE} ${SCANNER_DATA_FILE} ${OBJS} ${GLOBALDEFS} ${MAIN}
	${CPP} ${SCANNER_CONFIG_FILE} ${SCANNER_DATA_FILE} ${OBJS} ${MAIN} ${CPPFLAGS} ${LDFLAGS} -o $@

${OBJS}:  ${HEADERS} ${CLASSES}
	${CPP} -c ${CLASSES}

${SCANNER_CONFIG_FILE}:  ${GRAMMAR_CONFIG_FILE}
	${SCANNER} ${SCANNER_CONFIG_OPTIONS} ${GRAMMAR_CONFIG_FILE}

${SCANNER_DATA_FILE}:  ${GRAMMAR_DATA_FILE}
	${SCANNER} ${SCANNER_DATA_OPTIONS} ${GRAMMAR_DATA_FILE}

clean:
	${RM} -f *.o ${TARGET} ${SCANNER_CONFIG_FILE} ${SCANNER_CONFIG_HEADER} ${SCANNER_DATA_FILE} ${SCANNER_DATA_HEADER} *~
