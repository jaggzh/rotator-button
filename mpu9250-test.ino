// #define DO_RUN_CALIBRATION_EVAL

/* Axis of acceleration
 * (we use x axis, but if board orientation is changed... */
#define axis_acc  ax

#include "Wire.h"

// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"
#include "BMP180.h"
#include "ringbuffer/trb.h"

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU9250 accelgyro;
I2Cdev	 I2C_M;
uint8_t buffer_m[6];
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t   mx, my, mz;
float heading;
float tiltheading;
float Axyz[3];
float Gxyz[3];
float Mxyz[3];

#define sample_num_mdate  5000

volatile float mx_sample[3];
volatile float my_sample[3];
volatile float mz_sample[3];

static float mx_centre = 0;
static float my_centre = 0;
static float mz_centre = 0;

volatile int mx_max = 0;
volatile int my_max = 0;
volatile int mz_max = 0;

volatile int mx_min = 0;
volatile int my_min = 0;
volatile int mz_min = 0;

float temperature;
float pressure;
float atm;
float altitude;
//BMP180 Barometer;

rb_st rb_real;
rb_st *rb = &rb_real;
float slow_mn=0, slow_mx=0;
rb_st rb_real; rb_st *rb = rb_real;

void update_sensor_data() {
	getAccel_Data();
	getGyro_Data();
	getCompassDate_calibrated(); // compass data has been calibrated here
	getHeading();				//before we use this function we should run 'getCompassDate_calibrated()' frist, so that we can get calibrated data ,then we can get correct angle .
	getTiltHeading();
}

void setup() {
	// join I2C bus (I2Cdev library doesn't do this automatically)
	Wire.begin();

	// initialize serial communication
	// (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
	// it's really up to you depending on your project)
	Serial.begin(115200);
	delay(2000);

	// initialize device
	spl("Initializing I2C devices...");
	accelgyro.initialize();
	//Barometer.init();

	// verify connection
	spl("Testing device connections...");
	spl(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");

	delay(1000);
	spl("	 ");

	mx_centre = 13.00;
	my_centre = 2.00;
	mz_centre = -7.00;

	#ifdef DO_RUN_CALIBRATION_EVAL
		/* Mxyz_init_calibrated (); */
		// Returned: 13.00	   2.00		-7.00
	#endif

	spl("calibration parameters: ");
	sp(mx_centre);
	sp("		   ");
	sp(my_centre);
	sp("		   ");
	spl(mz_centre);
	spl("	 ");
	update_sensor_data();
	rb_setall(rb, axis_acc);
	slow_mn = axis_acc;
	slow_mx = axis_acc;

#define RB_BUF_SIZE 10
#define RB_WIN 5 // make this odd please.
	rb_init(rb, RB_BUF_SIZE);
	rb_setall(rb, axis_acc);
}

void loop() {
	#ifdef DO_RUN_CALIBRATION_EVAL
		spl("Done. restart without Mxyz_init_calibrated()");
		delay(5000);
		return;
	#endif

	update_sensor_data();
	rb_add(rb, axis_acc);
	rb_minmax(rb);
	slow_mn += (rb->mn - slow_mn)/64;
	slow_mx += (rb->mx - slow_mx)/64;

	//Serial.println("Acceleration(g) of X,Y,Z:");
	#define sp(v) Serial.print(v)
	#define spl(v) Serial.println(v)
	sp("ax:");  sp(ax);
	sp("smn:"); sp(slow_mn);
	sp("smx:"); sp(slow_mx);
	/* Serial.print(" ay:"); */
	/* Serial.print(ay); */
	/* Serial.print(" az:"); */
	/* Serial.print(az); */
	/* Serial.println("Gyro(degress/s) of X,Y,Z:"); */
	/* Serial.print(Gxyz[0]); */
	/* Serial.print(","); */
	/* Serial.print(Gxyz[1]); */
	/* Serial.print(","); */
	/* Serial.println(Gxyz[2]); */
	/* Serial.println("Compass Value of X,Y,Z:"); */
	/* sp("compX:"); */
	/* Serial.print(Mxyz[0]); */
	/* sp(" compy:"); */
	/* Serial.print(Mxyz[1]); */
	/* sp(" compz:"); */
	/* Serial.print(Mxyz[2]); */
	/* sp(" CWAng-North-to-Xaxis:"); */
	// Serial.println("The clockwise angle between the magnetic north and X-Axis:");
	/* Serial.print(heading); */
	/* sp(" CWAng-MagNorth-to-Xaxis_dot_horzPlane:"); */
	/* Serial.println("The clockwise angle between the magnetic north and the projection of the positive X-Axis in the horizontal plane:"); */
	/* Serial.print(tiltheading); */
	/* Serial.println("   "); */

	//temperature = Barometer.bmp180GetTemperature(Barometer.bmp180ReadUT()); //Get the temperature, bmp180ReadUT MUST be called first
	//pressure = Barometer.bmp180GetPressure(Barometer.bmp180ReadUP());//Get the temperature
	//altitude = Barometer.calcAltitude(pressure); //Uncompensated caculation - in Meters
	//atm = pressure / 101325;
	/*
	sp("Temperature: ");
	sp(temperature, 2); //display 2 decimal places
	spl("deg C");

	sp("Pressure: ");
	sp(pressure, 0); //whole number only.
	spl(" Pa");

	sp("Ralated Atmosphere: ");
	spl(atm, 4); //display 4 decimal places

	sp("Altitude: ");
	sp(altitude, 2); //display 2 decimal places
	spl(" m");
	*/
	spl();
	delay(25);

}


void getHeading(void)
{
	heading = 180 * atan2(Mxyz[1], Mxyz[0]) / PI;
	if (heading < 0) heading += 360;
}

void getTiltHeading(void)
{
	float pitch = asin(-Axyz[0]);
	float roll = asin(Axyz[1] / cos(pitch));

	float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
	float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
	float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
	tiltheading = 180 * atan2(yh, xh) / PI;
	if (yh < 0)    tiltheading += 360;
}



void Mxyz_init_calibrated ()
{

	spl(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
	sp("	");
	spl(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
	sp("	");
	spl(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
	while (!Serial.find("ready"));
	spl("  ");
	spl("ready");
	spl("Sample starting......");
	spl("waiting ......");

	get_calibration_Data ();

	spl("	 ");
	spl("compass calibration parameter ");
	sp(mx_centre);
	sp("	   ");
	sp(my_centre);
	sp("	   ");
	spl(mz_centre);
	spl("	");
}


void get_calibration_Data ()
{
	for (int i = 0; i < sample_num_mdate; i++)
	{
		get_one_sample_date_mxyz();
		/*
		sp(mx_sample[2]);
		sp(" ");
		sp(my_sample[2]);							   //you can see the sample data here .
		sp(" ");
		spl(mz_sample[2]);
		*/
		if (mx_sample[2] >= mx_sample[1])mx_sample[1] = mx_sample[2];
		if (my_sample[2] >= my_sample[1])my_sample[1] = my_sample[2]; //find max value
		if (mz_sample[2] >= mz_sample[1])mz_sample[1] = mz_sample[2];

		if (mx_sample[2] <= mx_sample[0])mx_sample[0] = mx_sample[2];
		if (my_sample[2] <= my_sample[0])my_sample[0] = my_sample[2]; //find min value
		if (mz_sample[2] <= mz_sample[0])mz_sample[0] = mz_sample[2];

	}

	mx_max = mx_sample[1];
	my_max = my_sample[1];
	mz_max = mz_sample[1];

	mx_min = mx_sample[0];
	my_min = my_sample[0];
	mz_min = mz_sample[0];

	mx_centre = (mx_max + mx_min) / 2;
	my_centre = (my_max + my_min) / 2;
	mz_centre = (mz_max + mz_min) / 2;
}

void get_one_sample_date_mxyz()
{
	getCompass_Data();
	mx_sample[2] = Mxyz[0];
	my_sample[2] = Mxyz[1];
	mz_sample[2] = Mxyz[2];
}


void getAccel_DataRaw(void)
{
	accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
}

void getAccel_Data(void)
{
	getAccel_DataRaw();
	/* \/ Moved to /\ */
	// accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
	Axyz[0] = (double) ax / 16384;
	Axyz[1] = (double) ay / 16384;
	Axyz[2] = (double) az / 16384;
}

void getGyro_Data(void)
{
	getAccel_DataRaw();
	Gxyz[0] = (double) gx * 250 / 32768;
	Gxyz[1] = (double) gy * 250 / 32768;
	Gxyz[2] = (double) gz * 250 / 32768;
}

void getCompass_Data(void)
{
	I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
	delay(10);
	I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);

	mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
	my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
	mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;

	Mxyz[0] = (double) mx * 1200 / 4096;
	Mxyz[1] = (double) my * 1200 / 4096;
	Mxyz[2] = (double) mz * 1200 / 4096;
}

void getCompassDate_calibrated ()
{
	getCompass_Data();
	Mxyz[0] = Mxyz[0] - mx_centre;
	Mxyz[1] = Mxyz[1] - my_centre;
	Mxyz[2] = Mxyz[2] - mz_centre;
}
