
#include <stdio.h>
#include <i2c.h>

int main(int args, char *argv[], char *environ[])
{
	while (1)
	{
		printf("hello\n");
	}
	return 0;
}
  BaudRate      = 38400         //Baud
  RxPin         = 31            //For RS232
  TxPin         = 30            //For RS232
  CommsMode     = %0000         //See SerialMirror.spin for meaning

  SCL = 28
  PDel = 2400

  Comp = 0x3C
  Acc = 0xA6
  Gyro = 0xD0

  DATA = 27
  CLK  = 26
  RDY  = 25

{
//Setup Compass

  //75Hz, Normal measurement
  i2c.Start(SCL)
  i2c.Write(SCL, Comp)
  i2c.Write(SCL, $00)
  i2c.Write(SCL, $18)
  i2c.Stop(SCL)
  //range of +/- 0.88Ga
  i2c.Start(SCL)
  i2c.Write(SCL, Comp)
  i2c.Write(SCL, $01)
  i2c.Write(SCL, $00)
  i2c.Stop(SCL)


//Setup Accelerometer

  //Data Format Register, set for max range (16g)
  i2c.Start(SCL)
  i2c.Write(SCL, Acc)
  i2c.Write(SCL, $31)
  i2c.Write(SCL, $00)
  i2c.Stop(SCL)
  //Power Control Register, set to measure
  i2c.Start(SCL)
  i2c.Write(SCL, Acc)
  i2c.Write(SCL, $2D)
  i2c.Write(SCL, $08)
  i2c.Stop(SCL)


//Setup Gyro

  //1000 samples per second,
  i2c.Start(SCL)
  i2c.Write(SCL, Gyro)
  i2c.Write(SCL, $15)
  i2c.Write(SCL, $07)
  i2c.Stop(SCL)
  //Set Range to 2000 deg/s
  i2c.Start(SCL)
  i2c.Write(SCL, Gyro)
  i2c.Write(SCL, $16)
  i2c.Write(SCL, $24)
  i2c.Stop(SCL)

}

void readSensors()
{
//Read Compass

  //Prep for 1 pass measurement
  i2c.Start(SCL)
  i2c.Write(SCL, Comp)
  i2c.Write(SCL, $02)
  i2c.Write(SCL, $01)
  i2c.Stop(SCL)
  //Measure
  i2c.Start(SCL)
  i2c.Write(SCL, Comp)
  i2c.Write(SCL, $03)
  i2c.Start(SCL)
  i2c.Write(SCL, $3D)
  CX := i2c.Read(SCL, 0)
  CX <-= 8
  CX := CX + i2c.Read(SCL, 0)
  CY := i2c.Read(SCL, 0)
  CY <-= 8
  CY := CY + i2c.Read(SCL, 0)
  CZ := i2c.Read(SCL, 0)
  CZ <-= 8
  CZ := CZ + i2c.Read(SCL, 1) //Final ACK bit set to 1, end read cycle
  i2c.Stop(SCL)

  CompX := CX
  CompY := CY
  CompZ := CZ

//Read Accelerometer
  i2c.Start(SCL)
  i2c.Write(SCL, Acc)
  i2c.Write(SCL, $32)

  i2c.Start(SCL)
  i2c.Write(SCL, $A7)
  WorkVar := i2c.Read(SCL, 0)
  WorkVar <-= 8
  AX := WorkVar + i2c.Read(SCL, 0)
  WorkVar := i2c.Read(SCL, 0)
  WorkVar <-= 8
  AY := WorkVar + i2c.Read(SCL, 0)
  WorkVar := i2c.Read(SCL, 0)
  WorkVar <-= 8
  AZ := WorkVar + i2c.Read(SCL, 1) //Final ACK bit set to 1, end read cycle
  i2c.Stop(SCL)

  AccX := AX
  AccY := AY
  AccZ := AZ

//Read Gyro
  i2c.Start(SCL)
  i2c.Write(SCL, Gyro)
  i2c.Write(SCL, $1D)
  i2c.Stop(SCL)

  i2c.Start(SCL)
  i2c.Write(SCL, $D1)
  GX := i2c.Read(SCL, 0)
  GX <-= 8
  GX := GX + i2c.Read(SCL, 0)
  GY := i2c.Read(SCL, 0)
  GY <-= 8
  GY := GY + i2c.Read(SCL, 0)
  GZ := i2c.Read(SCL, 0)
  GZ <-= 8
  GZ := GZ + i2c.Read(SCL, 1) //Final ACK bit set to 1, end read cycle
  i2c.Stop(SCL)

  GyroX := GX
  GyroY := GY
  GyroZ := GZ

}
