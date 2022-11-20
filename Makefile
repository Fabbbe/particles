CC=c99
PACKAGES=sdl2 gl glew cglm
INCLUDE=$(shell pkg-config --cflags --libs $(PACKAGES)) -Isrc/
CFLAGS=--std=c99 -O2 $(INCLUDE) # -DNDEBUG
OUTFILE=particles

# Based
%.o: src/%.c
	$(CC) -c -o $@ $^ $(CFLAGS)

$(OUTFILE): src/main.c shader.o
	$(CC) -o $@ $^ $(CFLAGS)


run: $(OUTFILE)
	./$(OUTFILE)

clean:
	rm $(OUTFILE) *.o *.log

default: $(OUTFILE)

