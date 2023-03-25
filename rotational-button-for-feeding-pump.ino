// #define DO_RUN_CALIBRATION_EVAL
//#define DEBUG_SHOW_ALL_VALUES

/* Axis of acceleration
 * (we use x axis, but if board orientation is changed... */
#define axis_acc  az

/* MIN_ACC_THRESH:
 * don't let auto calibration tighten our envelope TOO much.
 * This prevents the top of the envelop from getting too close to the
 * bottom */
#define MIN_ACC_THRESH 270

#include "Wire.h"

// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"
#include "BMP180.h"
#define RB_DTYPE int16_t
#include "trb.h"
#include "tone.h"
#include <InputDebounce.h>

#define sp(v) Serial.print(v)
#define spl(v) Serial.println(v)

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

static InputDebounce ourBtn;

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

#define RB_SIZE	      7
#define RB_MED_WINDOW 7
rb_st rb_real;     rb_st *rb = &rb_real;
rb_st rb_med_real; rb_st *rb_med = &rb_med_real;
float fast_mn, fast_mx;
float slow_mn, slow_mx;
float slow_acc;

enum en_modez {
	MO_NORMAL=0,
	MO_CALQUICK,
	MO_CALQUICK_DONE,
} mode=MO_NORMAL;

void update_sensor_data(void);

void update_sensor_data(void) {
	getAccel_Data();
	/* getGyro_Data(); */
	/* getCompassDate_calibrated(); // compass data has been calibrated here */
	/* getHeading();				//before we use this function we should run 'getCompassDate_calibrated()' frist, so that we can get calibrated data ,then we can get correct angle . */
	/* getTiltHeading(); */
}

void ourbtn_released_cb(uint8_t pinIn) {
	(void)pinIn; // unused
	spl("Button released");
	if (mode == MO_CALQUICK_DONE) {
		mode = MO_NORMAL;
	}
}
void ourbtn_pressed_dur_cb(uint8_t pinIn, unsigned long dur) {
	(void)pinIn; // unused
	(void)dur;
	spl("Button pressed");
	if (mode == MO_NORMAL) {
		tone_freq_durms(440, 200);
		tone_freq_durms(480, 200);
		mode = MO_CALQUICK;
	}
}

void setup() {
	// initialize serial communication
	// (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
	// it's really up to you depending on your project)
	Serial.begin(115200);
	// join I2C bus (I2Cdev library doesn't do this automatically)
	Wire.begin();
	delay(500);
	tone_quick_freq(294);

	// Logic button output
	pinMode(PIN_BTN_TX, OUTPUT);
	digitalWrite(PIN_BTN_TX, HIGH);

	// initialize device
	spl("Initializing I2C devices...");
	accelgyro.initialize();
	//Barometer.init();

	// verify connection
	spl("Testing device connections...");
	spl(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");

	delay(200);
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

	rb_init(rb, RB_SIZE);
	rb_init(rb_med, RB_SIZE);
	rb_setall(rb, axis_acc);

	fast_mn = fast_mx = slow_mn = slow_mx = axis_acc;
	slow_acc = axis_acc;

	ourBtn.registerCallbacks(NULL, ourbtn_released_cb, ourbtn_pressed_dur_cb, NULL);
	ourBtn.setup(PIN_BTN_OURS, 20, InputDebounce::PIM_INT_PULL_UP_RES);

	PLAY_SHAVE();
	spl("Initialized. We're on our way");
}

#define DELAY_SAMPLE_US 10000
#define DELAY_BTN_TEST_US 10000
#define DELAY_PLOT_US   13000

void loop() {
	RB_DTYPE mv;
	unsigned long cmicros = micros();
	unsigned long cmillis = millis();
	static unsigned long last_loop_us = cmicros;
	static unsigned long last_sample_us = cmicros;
	static unsigned long last_plot_us = cmicros;
	static unsigned long last_inpdeb_us = cmicros;
	#ifdef DO_RUN_CALIBRATION_EVAL
		spl("Done. restart without Mxyz_init_calibrated()");
		delay(5000);
		return;
	#endif


	if (cmicros-last_inpdeb_us >= DELAY_BTN_TEST_US) {
		last_inpdeb_us = cmicros;
		ourBtn.process(cmillis);
	}
	if (cmicros-last_sample_us >= DELAY_SAMPLE_US) {
		last_sample_us = cmicros;

		update_sensor_data();
		rb_add(rb, axis_acc);
		rb_median(rb, rb_med, RB_MED_WINDOW); // median filter the whole buffer
		rb_minmax(rb_med);
		fast_mx += (rb_med->mx - fast_mx)/32;
		fast_mn += (rb_med->mn - fast_mn)/32;
		if (slow_mx < rb_med->mx) slow_mx += (rb_med->mx - slow_mx)/256;
		else                      slow_mx += (rb_med->mx - slow_mx)/4096;
		if (slow_mn > rb_med->mn) slow_mn += (rb_med->mn - slow_mn)/256;
		else                      slow_mn += (rb_med->mn - slow_mn)/4096;
		/* Threshold tests: */

		/* Minimum threshold above slow_min. Hard coded. */
		if (slow_mx < slow_mn + MIN_ACC_THRESH)
			slow_mx = slow_mn + MIN_ACC_THRESH;

		int16_t slow_range = slow_mx - slow_mn;

		/* At least 3 times the noise level (3 * mx-mn) */
		float swbase_new = slow_mn - (slow_range)*3;
		static float swbase = swbase_new;
		swbase += (swbase_new - swbase)/10000;

		int triglvl = (slow_mx-slow_mn)*.75 + slow_mn;
		if (triglvl - slow_mn < ((float)fast_mx-fast_mn)*3)
			triglvl = slow_mn + ((float)fast_mx-fast_mn)*3;
		int untriglvl = (slow_range)*.52 + slow_mn;

		mv = rb_get_last(rb_med, 0);
		slow_acc += (mv - slow_acc)/8;

		if (mode == MO_CALQUICK) {
			slow_mx = fast_mx;
			slow_mn = fast_mn;
			slow_range = slow_mx - slow_mn;
			swbase = slow_mn - (slow_range)*3;
			rb_setall(rb, mv);
			mode = MO_CALQUICK_DONE;
		}

		static int trigger = 0;
		if (slow_acc > triglvl) {
			if (!trigger) {
				tone_freq_durms(430, 40);
				SEND_TRIGGERED();
			}
			trigger = 1;
		} else if (slow_acc < untriglvl) {
			if (trigger) {
				tone_freq_durms(380, 30);
				SEND_RELEASED();
			}
			trigger = 0;
		}

		#ifdef DEBUG_SHOW_ALL_VALUES
			sp(" med:");  sp(mv);
			sp(" smed:"); sp(slow_acc);
			sp(" smn:");  sp(slow_mn);
			sp(" raw:");  sp(axis_acc);
			sp(" smx:");  sp(slow_mx);
			sp(" SW:");   sp(swbase + (trigger * slow_range/2));
			sp(" lvl:");  sp(triglvl);
			sp(" ulvl:"); sp(untriglvl);
			sp(" fmx:");  sp(fast_mx);
			sp(" fmn:");  sp(fast_mn);
			/* sp(" us:");  sp(cmicros-last_loop_us); */
			/* last_loop_us = cmicros; */
			spl("");
		/* #else */
		/* 	spl(digitalRead(PIN_BTN_OURS)); */
		#endif
	}

	/* sp(" mn:"); sp(rb->mn); */
	/* sp(" mx:"); sp(rb->mx); */
	/* for (int i=0; i<rb->sz; i++) { */
	/* 	Serial.print(" ["); */
	/* 	Serial.print(i); */
	/* 	Serial.print("]"); */
	/* 	Serial.print(rb->d[i]); */
	/* } */
	/* Serial.print("\n"); */
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
	//spl();
	/* delay(25); */

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
	/* accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz); */
	accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
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
