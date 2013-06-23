#include "imu.h"
#include "vector3d.h"

float dcmEst[3][3] =
{
{ 1, 0, 0 },
{ 0, 1, 0 },
{ 0, 0, 1 } }; //estimated DCM matrix

time_ms previous_time;

void imu_init(time_ms time)
{
	previous_time = time;
}

void imu_update(time_ms time, float accelerometer[], float magnetometer[],
		float gyrometer[])
{
	float Imag[3]; //I(b) vector according to magnetometer in body's coordinates
	time_ms elapsed_time = time - previous_time;
	int i;

	//---------------
	// I,J,K unity vectors of global coordinate system I-North,J-West,K-zenith
	// i,j,k unity vectors of body's coordinate system i-"nose", j-"left wing", k-"top"
	//---------------
	//			[I.i , I.j, I.k]
	// DCM =  	[J.i , J.j, J.k]
	//			[K.i , K.j, K.k]

	//---------------
	//Acelerometer
	//---------------
	//Accelerometer measures gravity vector G in body coordinate system
	//Gravity vector is the reverse of K unity vector of global system expressed in local coordinates
	//K vector coincides with the z coordinate of body's i,j,k vectors expressed in global coordinates (K.i , K.j, K.k)

	vector3d_normalize(accelerometer);
	//calculate correction vector to bring dcmEst's K vector closer to Acc vector (K vector according to accelerometer)
	float wA[3];
	vector3d_cross(dcmEst[2], accelerometer, wA); // wA = Kgyro x	 Kacc , rotation needed to bring Kacc to Kgyro

	//---------------
	//Magnetometer
	//---------------
	//calculate correction vector to bring dcmEst's I vector closer to Mag vector (I vector according to magnetometer)
	//in the absense of magnetometer let's assume North vector (I) is always in XZ plane of the device (y coordinate is 0)
	Imag[0] = sqrt(1 - dcmEst[0][2] * dcmEst[0][2]);
	Imag[1] = 0;
	Imag[2] = dcmEst[0][2];

	vector3d_cross(dcmEst[0], Imag, magnetometer); // wM = Igyro x Imag, roation needed to bring Imag to Igyro

	//---------------
	//dcmEst
	//---------------
	//gyro rate direction is usually specified (in datasheets) as the device's(body's) rotation
	//about a fixed earth's (global) frame, if we look from the perspective of device then
	//the global vectors (I,K,J) rotation direction will be the inverse

	for (i = 0; i < 3; i++)
	{
		gyrometer *= elapsed_time; //scale by elapsed time to get angle in radians
		//compute weighted average with the accelerometer correction vector
		gyrometer = (gyrometer + ACC_WEIGHT * wA[i] + MAG_WEIGHT * magnetometer)
				/ (1.0 + ACC_WEIGHT + MAG_WEIGHT);
	}

	dcm_rotate(dcmEst, gyrometer);

}

//rotate DCM matrix by a small rotation given by angular rotation vector w
//see http://gentlenav.googlecode.com/files/DCMDraft2.pdf
void dcm_rotate(float dcm[3][3], float w[3])
{
	//float W[3][3];
	//creates equivalent skew symetric matrix plus identity matrix
	//vector3d_skew_plus_identity((float*)w,(float*)W);
	//float dcmTmp[3][3];
	//matrix_multiply(3,3,3,(float*)W,(float*)dcm,(float*)dcmTmp);

	int i;
	float dR[3];
	//update matrix using formula R(t+1)= R(t) + dR(t) = R(t) + w x R(t)
	for (i = 0; i < 3; i++)
	{
		vector3d_cross(w, dcm[i], dR);
		vector3d_add(dcm[i], dR, dcm[i]);
	}

	//make matrix orthonormal again
	dcm_orthonormalize(dcm);
}

//bring dcm matrix in order - adjust values to make orthonormal (or at least closer to orthonormal)
void dcm_orthonormalize(float dcm[3][3])
{
	//err = X . Y ,  X = X - err/2 * Y , Y = Y - err/2 * X  (DCMDraft2 Eqn.19)
	float err = vector3d_dot((float*) (dcm[0]), (float*) (dcm[1]));
	float delta[2][3];
	vector3d_scale(-err / 2, (float*) (dcm[1]), (float*) (delta[0]));
	vector3d_scale(-err / 2, (float*) (dcm[0]), (float*) (delta[1]));
	vector3d_add((float*) (dcm[0]), (float*) (delta[0]), (float*) (dcm[0]));
	vector3d_add((float*) (dcm[1]), (float*) (delta[1]), (float*) (dcm[1]));

	//Z = X x Y  (DCMDraft2 Eqn. 20) ,
	vector3d_cross((float*) (dcm[0]), (float*) (dcm[1]), (float*) (dcm[2]));
	//re-normalization
	vector3d_normalize((float*) (dcm[0]));
	vector3d_normalize((float*) (dcm[1]));
	vector3d_normalize((float*) (dcm[2]));
}
