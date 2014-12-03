qr: main.c qr.c qr.h
	gcc --std=c99 -Wall -Werror -oqr main.c qr.c -lpng

test: qr_test.c qr.c qr.h
	gcc --std=c99 -Wall -Werror -oqr_test qr_test.c && ./qr_test

generate: qr test.txt
	./qr < test.txt > test.png
