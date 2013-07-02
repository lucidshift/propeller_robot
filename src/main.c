#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
#include <time.h>
#include <limits.h>

#define SCL_PIN 28
#define SDA_PIN 29
#define COMP_ADR 0x3C
#define ACC_ADR 0xA6
#define GYRO_ADR 0xD0

#define COMP_DIVISOR 1370

static void printVectorLine(char* name, int16_t *arr);
static void setupSensors();
static void readSensors();
static int16_t convert(uint8_t *raw);

I2C_COGDRIVER *driver;
int16_t *comp, *acc, *gyro;

int main(int args, char *argv[], char *environ[])
{
	driver = (I2C_COGDRIVER*) malloc(sizeof(I2C_COGDRIVER));
	i2cOpen(driver, SCL_PIN, SDA_PIN, 400000);

	comp = (int16_t*) malloc(3 * sizeof(int16_t));
	acc = (int16_t*) malloc(3 * sizeof(int16_t));
	gyro = (int16_t*) malloc(3 * sizeof(int16_t));

	clock_t start;
	clock_t end;
	clock_t elapsed;
	clock_t time;

	setupSensors();

	while (1)
	{
		start = clock();
		readSensors();
		end = clock();

		if (end > start)
		{
			elapsed = end - start;
		}
		else
		{
			elapsed = end + (UINT_MAX - start);
		}

		time = elapsed;
		time /= 4000;

		printf("%-*s: %d ms\n", 15, "time", time);

		printVectorLine("compass", comp);
		printVectorLine("accelerometer", acc);
		printVectorLine("gyrometer", gyro);
	}
	return 0;
}

static void printVectorLine(char* name, int16_t *arr)
{
	printf("%-*s: <%d, %d, %d>\n", 15, name, arr[0], arr[1], arr[2]);
}

static void setupSensors()
{
	uint8_t out[4];

	//Setup Compass

	//TODO: consider averaging values
	//TODO: possibly use default settings
	out[0] = 0x00; //Configuration Register A
	out[1] = 0x18; //75Hz, Normal measurement
	//register value automatically incremented to 0x01
	out[2] = 0x00; //range of +/- 0.88Ga

	//write 3 bytes to the compass
	i2cWrite(&driver->i2c, COMP_ADR, out, 3, 1);

	//Setup Accelerometer

	out[0] = 0x31; //Data Format Register
	out[1] = 0x00; //set for max range (16g)
	out[2] = 0x2d; //Power Control Register
	out[3] = 0x08; //set to measure

	// write first 2 bytes to the accelerometer
	i2cWrite(&driver->i2c, ACC_ADR, out, 2, 1);
	// write next 2 bytes to the accelerometer
	i2cWrite(&driver->i2c, ACC_ADR, &out[2], 2, 1);

	//Setup Gyro

	out[0] = 0x15; //Sample Rate Divider Register
	out[1] = 0x07; //divider value (1 is added, so really 8)
	//register value automatically incremented to 0x16
	out[2] = 0x1a;

	//write 3 bytes to the gyro
	i2cWrite(&driver->i2c, GYRO_ADR, out, 3, 1);
}

/*
 * The processor is little-endian. This converts big-endian bytes to little-endian
 * and packages the result in a two-byte-long integer.
 */
static int16_t convert(uint8_t *raw)
{
	union
	{
		uint8_t bytes[2];
		int16_t value;
	} temp;
	temp.bytes[0] = raw[1];
	temp.bytes[1] = raw[0];

	return temp.value;
}

static void readSensors()
{
	uint8_t out[10];
	uint8_t in[6];
	int i;

	//Read Compass
	//Prep for 1 pass measurement

	//TODO: the write may not be necessary; definitely isn't for continuous mode
	out[0] = 0x02; //Mode Register
	out[1] = 0x01; //reset
	//register value automatically incremented to 0x03

	//write 2 bytes to the compass
	i2cWrite(&driver->i2c, COMP_ADR, out, 2, 0);
	i2cRead(&driver->i2c, COMP_ADR, in, 6, 1);

	/*
	 * The compass stores values in big-endian and in xzy order.
	 * Conversions must be done.
	 */
	comp[0] = convert(in); //x
	comp[1] = convert(&in[4]); //y
	comp[2] = convert(&in[2]); //z

	//Read Accelerometer

	out[0] = 0x32; //first data register

	i2cWrite(&driver->i2c, ACC_ADR, out, 1, 0);
	i2cRead(&driver->i2c, ACC_ADR, in, 6, 1);

	/*
	 * The accelerometer stores values in little-endian and xyz order.
	 * No conversions need to be done.
	 */
	for (i = 0; i < 3; i++)
	{
		acc[i] = (int8_t) in[2 * i];
	}

	//Read Gyro

	out[0] = 0x1d; //first data register.

	i2cWrite(&driver->i2c, GYRO_ADR, out, 1, 0);
	i2cRead(&driver->i2c, GYRO_ADR, in, 6, 1);

	/*
	 * The gyro stores values in big-endian and xyz order.
	 * Conversions must be done.
	 */
	gyro[0] = convert(in); //x
	gyro[1] = convert(&in[2]); //y
	gyro[2] = convert(&in[4]); //z
}
