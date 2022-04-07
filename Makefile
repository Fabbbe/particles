CC=c99
CFLAGS=$(shell pkg-config --libs sdl2 gl glew cglm) -Isrc/ -O2# -DNDEBUG
OUTFILE=particles

# Based
%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUTFILE): src/main.c shader.o
	$(CC) $(CFLAGS) -o $@ $^


run: $(OUTFILE)
	./$(OUTFILE)

clean:
	rm $(OUTFILE) *.o *.log

default: $(OUTFILE)

