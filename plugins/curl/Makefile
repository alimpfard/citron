
name = curl

all:
	gcc -g -c $(name).c -Wall -Wno-unused-function -Werror -fpic -o $(name).o
	gcc -shared -o libctr$(name).so $(name).o -l$(name)
	cp libctr$(name).so ../../mods/curl/
