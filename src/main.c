#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>

#define SCL_PIN 28
#define COMP_ADR 0x3C
#define ACC_ADR 0xA6
#define GYRO_ADR 0xD0

I2C *dev;
int16_t *comp, *acc, *gyro;

int main(int args, char *argv[], char *environ[])
{
	comp = (int16_t*) malloc(3 * sizeof(int16_t));
	acc = (int16_t*) malloc(3 * sizeof(int16_t));
	gyro = (int16_t*) malloc(3 * sizeof(int16_t));

	while (1)
	{
		printf("hello\n");
	}
	return 0;
}

static void setup_sensors()
{
	uint8_t out[4];

	//Setup Compass

	//TODO: consider averaging values
	//TODO: possibly use default settings
	out[0] = 0x00; //Configuration Register B
	out[1] = 0x18; //75Hz, Normal measurement
	//register value automatically incremented to 0x01
	out[2] = 0x00; //range of +/- 0.88Ga

	//write 3 bytes to the compass
	i2cWrite(dev, COMP_ADR, out, 3, 1);

	//Setup Accelerometer

	out[0] = 0x31; //Data Format Register
	out[1] = 0x00; //set for max range (16g)
	out[2] = 0x2d; //Power Control Register
	out[3] = 0x08; //set to measure

	// write first 2 bytes to the accelerometer
	i2cWrite(dev, ACC_ADR, out, 2, 1);
	// write next 2 bytes to the accelerometer
	i2cWrite(dev, ACC_ADR, &out[2], 2, 1);

	//Setup Gyro

	out[0] = 0x15; //Sample Rate Divider Register
	out[1] = 0x07; //divider value (1 is added, so really 8)
	//register value automatically incremented to 0x16
	out[2] = 0x1a;

	//write 3 bytes to the gyro
	i2cWrite(dev, GYRO_ADR, out, 3, 1);
}

static int16_t convert(uint8_t *raw)
{
	uint8_t temp[2];
	temp[0] = raw[1];
	temp[1] = raw[0];

	return (int16_t) temp[0];
}

static void read_sensors()
{
	uint8_t out[10];
	uint8_t in[6];

	//Read Compass
	//Prep for 1 pass measurement

	//TODO: the write may not be necessary; definitely isn't for continuous mode
	out[0] = 0x02; //Mode Register
	out[1] = 0x01; //reset
	//register value automatically incremented to 0x03

	//write 2 bytes to the compass
	i2cWrite(dev, COMP_ADR, out, 2, 1);
	i2cRead(dev, COMP_ADR, in, 6, 1);

	comp[0] = convert(in); //x
	comp[1] = convert(&in[4]); //y
	comp[2] = convert(&in[2]); //z

	i2c.Start(SCL)
	i2c.Write(scl_pin, comp)
	i2c.Write(scl_pin, $02)
	i2c.Write(scl_pin, $01)
	i2c.Stop(scl_pin)
	//Measure
	i2c.Start(SCL)
	i2c.Write(scl_pin, comp)
	i2c.Write(scl_pin, $03)
	i2c.Start(scl_pin)
	i2c.Write(scl_pin, $3D)
CX := i2c.Read(scl_pin, 0)
CX <-= 8
CX := CX + i2c.Read(scl_pin, 0)
CY := i2c.Read(scl_pin, 0)
CY <-= 8
CY := CY + i2c.Read(scl_pin, 0)
CZ := i2c.Read(scl_pin, 0)
CZ <-= 8
CZ := CZ + i2c.Read(scl_pin, 1) //Final ACK bit set to 1, end read cycle
i2c.Stop(SCL)

CompX := CX
CompY := CY
CompZ := CZ

//Read Accelerometer
i2c.Start(SCL)
i2c.Write(scl_pin, accelerometer_address)
i2c.Write(scl_pin, $32)

i2c.Start(scl_pin)
i2c.Write(scl_pin, $A7)
WorkVar := i2c.Read(scl_pin, 0)
WorkVar <-= 8
AX := WorkVar + i2c.Read(scl_pin, 0)
WorkVar := i2c.Read(scl_pin, 0)
WorkVar <-= 8
AY := WorkVar + i2c.Read(scl_pin, 0)
WorkVar := i2c.Read(scl_pin, 0)
WorkVar <-= 8
AZ := WorkVar + i2c.Read(scl_pin, 1)//Final ACK bit set to 1, end read cycle
i2c.Stop(SCL)

AccX := AX
AccY := AY
AccZ := AZ

//Read gyro_address
i2c.Start(SCL)
i2c.Write(scl_pin, Gyro)
i2c.Write(scl_pin, $1D)
i2c.Stop(scl_pin)

i2c.Start(scl_pin)
i2c.Write(scl_pin, $D1)
GX := i2c.Read(scl_pin, 0)
GX <-= 8
GX := GX + i2c.Read(scl_pin, 0)
GY := i2c.Read(scl_pin, 0)
GY <-= 8
GY := GY + i2c.Read(scl_pin, 0)
GZ := i2c.Read(scl_pin, 0)
GZ <-= 8
GZ := GZ + i2c.Read(scl_pin, 1)//Final ACK bit set to 1, end read cycle
i2c.Stop(SCL)

GyroX := GX
GyroY := GY
GyroZ := GZ

}
