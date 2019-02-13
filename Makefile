huffman : huffman.o main.o huffman.h
	cc -o huffman -g huffman.o main.o

.PHONY :clean

clean:
	-rm *.o *.comp *.decomp
