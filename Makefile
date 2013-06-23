CC=/opt/parallax/bin/propeller-elf-gcc
MODEL=lmm
CFLAGS=-m$(MODEL) -Wall -Os -m32bit-doubles -lm -Iinclude

all: bin/main.o bin/imu.o
	$(CC) -o bin/main.elf $^ $(CFLAGS)

bin/main.o: src/main.c
	$(CC) -c src/main.c -o bin/main.o $(CFLAGS)
	
bin/imu.o: src/imu.c
	$(CC) -c src/imu.c -o bin/imu.o $(CFLAGS)
	
clean:
	rm -f bin/Debug/*.o bin/Debug/*.elf
