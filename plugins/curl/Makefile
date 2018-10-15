name = curl

INTO ?= ../../mods/curl

all: build install

build:
	${CC} -g ${EXTRAS} -c $(name).c -Wall -Wno-unused-function -fpic -o $(name).o
	${CC} -shared -o libctr$(name).so $(name).o -l$(name)

install:
	mkdir -p $(INTO)
	cp libctr$(name).so $(INTO)
