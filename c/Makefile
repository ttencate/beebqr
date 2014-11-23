qr: *.c *.h
	gcc --std=c99 -Wall -Werror -oqr *.c -lpng

.PHONY: test
test: qr test.txt
	./qr < test.txt > test.png
