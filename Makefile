huffman : huffman.o main.o
	cc -o huffman -g huffman.o main.o

.PHONY :clean

clean:
	-rm *.o *.comp *.decomp
