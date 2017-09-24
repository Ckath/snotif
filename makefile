# see LICENSE file for copyright and license details.

VERSION = 0.3
DEST = /usr/local
NAME=snotif
CC=gcc
FLAGS=`pkg-config --cflags --libs libnotify` -Os -Wall -DVERSION=\"${VERSION}\"
DEPS = config.h
OBJ = ${NAME}.o

options:
	@echo ${NAME} build options:
	@echo "FLAGS    = ${FLAGS}"
	@echo "CC       = ${CC}"

%.o: %.c ${DEPS}
	@echo CC $<
	@${CC} -c ${FLAGS} $<

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

${NAME}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ $^ ${FLAGS}

clean:
	@echo cleaning
	@rm -f ${NAME} *.o

install: ${NAME}
	@echo installing executable file to ${DEST}/bin
	@mkdir -p ${DEST}/bin
	@cp -f ${NAME} ${DEST}/bin
	@chmod 755 ${DEST}/bin/${NAME}

uninstall: ${NAME}
	@echo removing executable file from ${DEST}/bin
	@rm -f ${DEST}/bin/${NAME}
