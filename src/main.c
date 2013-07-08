#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
#include <time.h>
#include <limits.h>
#include <imu.h>

#define SCL_PIN 28
#define SDA_PIN 29
#define COMP_ADR 0x3C
#define ACC_ADR  0xA6
#define GYRO_ADR 0xD0

#define COMP_DIVISOR 1370.0  //LSB/gauss
#define ACC_DIVISOR  32.0    //LSB/g
#define GYRO_DIVISOR 14.375  //LSB/degrees per second
static void printVectorLine(char* name, float *arr);
static void printMatrixLine(char* name, float matrix[3][3]);
static void setupSensors();
static void readSensors();
static int16_t convert(uint8_t *raw);

I2C_COGDRIVER *driver;
float *comp, *acc, *gyro;

int main(int args, char *argv[], char *environ[])
{
	driver = (I2C_COGDRIVER*) malloc(sizeof(I2C_COGDRIVER));
	i2cOpen(driver, SCL_PIN, SDA_PIN, 400000);

	comp = (float*) malloc(3 * sizeof(float));
	acc = (float*) malloc(3 * sizeof(float));
	gyro = (float*) malloc(3 * sizeof(float));

	clock_t previous;
	clock_t current;
	clock_t cyclesElapsed;
	time_ms timeElapsed;

	setupSensors();

	previous = clock();

	while (1)
	{
		readSensors();
		current = clock();

		if (current > previous)
		{
			cyclesElapsed = current - previous;
		}
		else
		{
			cyclesElapsed = current + (UINT_MAX - previous);
		}

		timeElapsed = (time_ms) cyclesElapsed;
		timeElapsed /= 4000;

		//imu_update(timeElapsed, acc, comp, gyro);

		printVectorLine("compass", comp);
		printVectorLine("accelerometer", acc);
		printVectorLine("gyrometer", gyro);
		printMatrixLine("dcm", dcmEst);

		previous = current;
	}
	return 0;
}

static void printVectorLine(char* name, float *arr)
{
	printf("%-*s: <%f, %f, %f>\n", 15, name, (double) arr[0], (double) arr[1], (double) arr[2]);
}

static void printMatrixLine(char* name, float matrix[3][3])
{
	//TODO: revise this
	printf("%-*s: [<%f, %f, %f> <%f, %f, %f> <%f, %f, %f>]\n", 15, name,
			(double) matrix[0][0], (double) matrix[0][1], (double) matrix[0][2],
			(double) matrix[1][0], (double) matrix[1][1], (double) matrix[1][2],
			(double) matrix[2][0], (double) matrix[2][1],
			(double) matrix[2][2]);
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

//compass: 6 ms, acc and gyro: each 5 ms
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
	comp[0] = (float) convert(in); //x
	comp[1] = (float) convert(&in[4]); //y
	comp[2] = (float) convert(&in[2]); //z

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
		//TODO: check this over..
		acc[i] = (float) *((int16_t*) &in[2 * i]);
	}

	//Read Gyro

	out[0] = 0x1d; //first data register.

	i2cWrite(&driver->i2c, GYRO_ADR, out, 1, 0);
	i2cRead(&driver->i2c, GYRO_ADR, in, 6, 1);

	/*
	 * The gyro stores values in big-endian and xyz order.
	 * Conversions must be done.
	 */
	gyro[0] = (float) convert(in); //x
	gyro[1] = (float) convert(&in[2]); //y
	gyro[2] = (float) convert(&in[4]); //z

	//convert to actual units
	for (i = 0; i < 3; i++)
	{
		comp[i] /= COMP_DIVISOR;
		acc[i] /= ACC_DIVISOR;
		gyro[i] /= GYRO_DIVISOR;
	}
}
