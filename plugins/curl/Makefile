
name = curl

all:
	${CC} -g ${EXTRAS} -c $(name).c -Wall -Wno-unused-function -fpic -o $(name).o
	${CC} -shared -o libctr$(name).so $(name).o -l$(name)
	mkdir -p ../../mods/curl
	cp libctr$(name).so ../../mods/curl/
