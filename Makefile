CC=/opt/parallax/bin/propeller-elf-gcc
DEPS=
OBJ=bin/main.o
CFLAGS=-m$(MODEL) -Wall -Os -m32bit-doubles -lm
MODEL=lmm

bin/%.o: %.c $(DEPS)
	$(CC) -c -o bin/$@ $< $(CFLAGS)

Debug: $(OBJ)
	$(CC) -o bin/Debug/main.elf $^ $(CFLAGS)

cleanDebug:
	rm -f bin/Debug/*.o bin/Debug/*.elf
