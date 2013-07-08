#ifndef IMU__H
#define IMU__H

#define ACC_WEIGHT_MAX 0.04			//maximum accelerometer weight in accelerometer-gyro fusion formula
//this value is tuned-up experimentally: if you get too much noise - decrease it
//if you get a delayed response of the filtered values - increase it
//starting with a value of  0.01 .. 0.05 will work for most sensors

#define ACC_WEIGHT 0.01		//accelerometer data weight relative to gyro's weight of 1
#define ACC_ERR_MAX 0.3		//maximum accelerometer error relative to 1g , when error exceeds this value accelerometer weight becomes 0
//this helps reject external accelerations (non-gravitational inertial forces caused by device acceleration)
#define MAG_WEIGHT 0.0		//magnetometer data weight relative to gyro's weight of 1
#define MAG_ERR_MAX 0.3		//maximum magnetometer error relative to normal value of 1 (corresponding to earth's magnetic field)
//when error exceeds this value magnetometer weight becomes 0
//this helps reject magnetic forces that are not attributed to earth's magnetic field

typedef float time_ms;

float dcmEst[3][3];

void imu_update(time_ms time, float *accelerometer, float *magnetometer,
		float *gyrometer);

void dcm_rotate(float dcm[3][3], float w[3]);

void dcm_orthonormalize(float dcm[3][3]);

#endif
