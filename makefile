# see LICENSE file for copyright and license details.
# TODO: write actual manpages

VERSION = 0.2
PREFIX = /usr/local
NAME=snotif
CC=gcc
FLAGS=-pthread -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -Wall -DVERSION=\"${VERSION}\"
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
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}
	@# @echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@# @mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@# @sed "s/VERSION/${VERSION}/g" < ${NAME}.1 > ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
	@# @chmod 644 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

uninstall: ${NAME}
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	@# @echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@# @rm -f ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
