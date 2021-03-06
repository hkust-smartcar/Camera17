#include <cassert>
#include <cstring>
#include <string>
#include <sstream>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>
#include <libsc/led.h>
#include <libsc/button.h>
#include <libbase/k60/gpio.h>
#include <libsc/st7735r.h>
#include <libsc/lcd.h>
#include <libsc/k60/ov7725.h>
#include "libsc/k60/ov7725_configurator.h"
#include <libsc/futaba_s3010.h>
#include <libsc/servo.h>
#include <libsc/alternate_motor.h>
#include <libsc/dir_encoder.h>
#include <libsc/lcd_typewriter.h>
#include <libsc/k60/jy_mcu_bt_106.h>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <libsc/debug_console.h>
#include <libsc/joystick.h>
#define CAM_W 80
#define CAM_H 60
#define cam_w 80
#define cam_h 120
float KP  =2;
float KD  =0 ;
float KP_curve = 6.5;
float KD_curve = 0;
float KP_curveless = 5;
float KD_curveless = 0;
float KP_cross = 3;
float KD_cross = 0;
float KP_R  =6;
float KD_R  =0;
float KP_LM = 8;
float KD_LM = 0.1;
float KI_LM = 0.1;
float KP_RM =8;
float KD_RM =0.1;
float KI_RM =0.1;
float KP_obstacle = 1.9;
float KD_obstacle = 0;
float initial_servo =830;
float speed =2.5;
float speed_ratio =1;
float ratio_ratio =1;
float increase_ratio =1;
float double_corner = 1;
float roundabout_ratio = 1;
float curve_ratio = 1;
float  crossroad_ratio = 1;
float entercross_ratio = 0.95;
float obstacle_ratio = 1;
float KP_C = 6.5;
float KD_C = 0;
#define L(y) y
#define R(y) y+60
int range =280;
float middleline = 40;
float offset =23;
float round_set =28;
float test_angle = 830;
float L_curvature1 = 0;
float R_curvature1 = 0;
float L_curvature2 = 0;
float R_curvature2 = 0;
int32_t L_motor_error = 0;
int32_t L_premotor_error = 0;
int32_t L_motor_errorsum=0;
int16_t L_motor_ideal = 0;

int32_t R_motor_error = 0;
int32_t R_premotor_error = 0;
int32_t R_motor_errorsum = 0;
int16_t R_motor_ideal = 0;


int32_t L_encoder =0;
int32_t R_encoder =0;
int32_t L_encoder_count_test = 0;
int32_t R_encoder_count_test = 0;
int32_t L_encoder_count = 0;
int32_t R_encoder_count = 0; // 5800 corresponding to 1 meter.  512-> 1 cycle->8.826 cm
int8_t L_power = 0;
int8_t R_power = 0;
int8_t num_check = 4;
namespace libbase
{
	namespace k60
	{

		Mcg::Config Mcg::GetMcgConfig()
		{
			Mcg::Config config;
			config.external_oscillator_khz = 50000;
			config.core_clock_khz = 200000;
			return config;
		}

	}
}

using namespace libsc;
using namespace libbase::k60;
using namespace std;
using namespace libsc::k60;

//BT listener
string inputStr;
bool tune = false;
vector<double> constVector;





bool programRun = false;
struct set{
	int8_t edgeposition; //x-coordinate
	float slope;
	int8_t row;// y-coordinate

};
set edge[120];
int width[60];
bool left_bottom_exist = false;
bool right_bottom_exist = false;

int servo_variable = 0;
float ServoErr = 0;
int ServoPreErr = 0;
int Left_x[15];
int Right_x[15];
int idealdegree = initial_servo;
int pre_idealdegree = 0;
int pre_state = 0;
int32_t L_ideal_count = 0;
int32_t R_ideal_count = 0;
int	num_of_round = 0;
int num_of_cross = 0;
int num_of_obstacle = 0;
bool corner_Lexist = false;
bool corner_Rexist = false;
bool L_slow = false;
bool R_slow = false;
bool crossroad = false;
bool entercross =false;
bool Rcross = false;
bool roundabout = false;
bool obstacle = false;
bool L_roundabout = false;
bool R_roundabout = false;
bool obstacle_left = false;
bool obstacle_right = false;
bool Round_step = false;
bool Round_step2 = false;
bool Cross_step = false;
bool Lcurve = false;
bool Rcurve = false;
bool Ltran = false;
bool Rtran = false;
bool Lcurve_less = false;
bool Rcurve_less = false;
bool Lcurve_mid = false;
bool Rcurve_mid = false;
bool sum_of_encoder = false;
bool record = false;
bool record1 = false;
bool motor_run = true;
bool run = false;
bool start = false;
bool manual = false;
bool lcd_read = false;
bool L_far_corner = false;
bool R_far_corner = false;
bool camptr[CAM_W][CAM_H];

//int8_t means integer with 8 bits
//Uint means unsigned integer
/*
 * A function extract the byte array
 * @ camBuffer: initial camera array in Byte
 * @ return type: bool array in 0/1 - only one bit
 * NOTE: Inside the camera, one pixel = one bit
 */
void extract_cam (const Byte* camBuffer) {
	Uint pos = 300;
	int bit_pos = 8;

for(Uint i=0; i < 40; i++){
	for(Uint j = 0;j<CAM_W ; j++){
		if (--bit_pos < 0) // Update after 8 bits are read
					{
						bit_pos = 7;// to track which position in a branch of Byte(Totally 8) is being read now.
						++pos;// to track which position in Byte array is being read now.
					}
					camptr[j][i] = GET_BIT(camBuffer[pos], bit_pos);
	}
}
pos = pos -10;
for(Uint i =40;i<60;i++){
	pos += 10;
	for(Uint j = 0;j<CAM_W ; j++){
		if (--bit_pos < 0) // Update after 8 bits are read
					{
						bit_pos = 7;// to track which position in a branch of Byte(Totally 8) is being read now.
						++pos;// to track which position in Byte array is being read now.
					}
					camptr[j][i] = GET_BIT(camBuffer[pos], bit_pos);
	}
}
}
Byte imgBuffer[600];
void compress_cam(){
	Uint pos = 0;
	int byt = 0;
for (Uint i=0; i< CAM_H;i++){
	for (Uint j=0; j<CAM_W;j += 8){
		byt = (128*camptr[j][i]+64*camptr[j+1][i]+32*camptr[j+2][i]+16*camptr[j+3][i]+8*camptr[j+4][i]+4*camptr[j+5][i]+2*camptr[j+6][i]+camptr[j+7][i]);
		imgBuffer[pos] = byt;
		pos++;
	}
}
}




void Filter2D(){
	for (int j = 1; j < CAM_H-1; j++){
		for (int i = 1; i < CAM_W-1; i++){
			int count = 0;
			for (int y = j-1; y < j+2; y++){
				for (int x = i-1; x < i+2; x++){
					count += (int)camptr[x][y];
				}
			}
			if (count >= 5) {
				camptr[i][j] = 1;
			} else {
				camptr[i][j] = 0;
			}
		}
	}
}

St7735r* lcdP;
AlternateMotor* L_motorP;
AlternateMotor* R_motorP;
FutabaS3010* servoP;
LcdTypewriter* writerP;



St7735r::Config getLcdConfig()
{
	St7735r::Config c;
	return c;
}

FutabaS3010::Config getServoConfig(){
	FutabaS3010::Config c;
	c.id =0;
	return c;
}

k60::Ov7725::Config getCameraConfig()
{
	k60::Ov7725::Config c;
	c.id = 0;
	c.w = CAM_W;
	c.h = cam_h;
	c.fps = k60::Ov7725Configurator::Config::Fps::kHigh;
	return c;
}

Led::Config getLedConfig(){
	Led::Config c;
	c.id = 0;
	c.is_active_low = false;
	return c;

}


//int8_t K[60];
//int8_t J[60][80];

/*
 * Find corresponding world coordinate to the camera.
 *
 *
 *
 *
 *
 *
 *
 *
 * */
//void Change_Matrix(){
//	for(int y=0; y < 60 ;y++){
//		K[y] = int((height*(y-2.1) + height*focalL*0.446 + 40*(y-2.1))/(focalL*0.899-(y-2.1)*0.446));
//		for(int x=0; x<80 ;x++){
//			J[y][x] = (int)((x-40)(K[y](0.446/focalL) + height*(0.899/focalL) +1 ));
//	}
//}
//}

struct coor{
	float x;
	float y;
};
coor L_corner;
coor R_corner;
std::vector<coor> Path;
std::vector<coor> detectline;



void Print2D(){

	for (int y=0; y<CAM_H; y++){
		for (int x=0; x<CAM_W; x++){
			lcdP->SetRegion(Lcd::Rect(x, y+1, 1, 1));
			if (!camptr[x][y]){
				lcdP->FillColor(0xFFFF);
			} else {
				lcdP->FillColor(0x001F);
			}
		}
	}

}
k60::Ov7725* cameraP;


void find_edge();//code for edge finding
void Printedge();
void printCameraImage(const Byte* image);
void turningPID(); // change the angle of servo
void motorPID(int ,int);
void checkround();
void findpath();
void Printpath();
bool Lcorner(int , int );
bool Rcorner(int , int);
bool roundabout_detect(int,int);
bool startline_detect();
bool obstacle_detect();
int func(int );
float curvature(set,set,set);
double angle(set,set,set);
void reset();
bool bluetoothListener(const Byte *data, const size_t size) {
	if(!manual){
		if (data[0] == 'w') {
			run = true;
			motor_run = true;
		}
	}
//	else{
//		if (data[0] == 'w') {
//			motorPID(1.5*117,servoP->GetDegree());
//		}
//		else if (data[0] == 's') {
//			motorPID(1.5*117,servoP->GetDegree(),false);
//		}
//		else{
//			motorPID(0,servoP->GetDegree());
//		}
//
//		if (data[0] == 'a') {
//			servo_variable += 30;
//			if(servo_variable > 250){
//				servo_variable = 250;
//			}
//		}
//
//		else if (data[0] == 'd') {
//			servo_variable -= 30;
//			if(servo_variable < -250){
//				servo_variable = -250;
//			}
//		}
//	}




	if (data[0] == 'P') {
		if(run){
			run = false;
		}
	}

	if (data[0] == 't') {
		programRun = 1;
		tune = 1;
		inputStr = "";
	}
	if (tune) {
		reset();
		unsigned int i = 0;
		while (i<size) {
			if (data[i] != 't' && data[i] != '\n') {
				inputStr += (char)data[i];
			} else if (data[i] == '\n') {
				tune = 0;
				break;
			}
			i++;
		}
		if (!tune) {
			constVector.clear();
			char * pch;
			pch = strtok(&inputStr[0], ",");
			while (pch != NULL){
				double constant;
				stringstream(pch) >> constant;
				constVector.push_back(constant);
				pch = strtok (NULL, ",");
			}


			 KP   = constVector[0];
			 KD   = constVector[1];
			 KP_curve = constVector[2];
			 KD_curve =  constVector[3];
			 KP_R  = constVector[4];
			 KD_R  = constVector[5];
			 KP_LM = constVector[6];
			 KD_LM = constVector[7];
			 KI_LM = constVector[8];
			 KP_RM = constVector[9];
			 KD_RM = constVector[10];
			 KI_RM = constVector[11];
			 initial_servo = constVector[12];
			 speed = constVector[13];
			 speed_ratio = constVector[14];
			 ratio_ratio = constVector[15];
			 increase_ratio = constVector[16];
			 double_corner =  constVector[17];
			 roundabout_ratio = constVector[18];
			 curve_ratio =  constVector[19];
			 crossroad_ratio =  constVector[20];
			 entercross_ratio = constVector[21];
			 obstacle_ratio = constVector[22];
			 KP_C = constVector[23];
			 KD_C = constVector[24];
			 KP_obstacle = constVector[25];
			 KD_obstacle = constVector[26];
			 range = constVector[29];
			 middleline = constVector[30];
			 KP_curveless = constVector[31];
			 KD_curveless = constVector[32];
		}
	}

//	else if (data[0] == 'a') {
//		servoPtr->SetDegree(900);
//	} else if (data[0] == 'd') {
//		servoPtr->SetDegree(430);
//	} else if (data[0] == 'A' || data[0] == 'D') {
//		servoPtr->SetDegree(700);
//	}

	return 1;
}
int main(void)
{

 	for(int i = 0;i<10; i++){
			width[i] = 46 - int(i*7/10);
		}
		for(int i = 10;i< 20;i++){
			width[i] = 39 - int((i-10)*9/10);
		}
		for(int i = 20; i< 30; i++){
			width[i] = 30 - (i-20);
		}
		for(int i =30; i< 40; i++){
			width[i] = 20 - int((i-30)*5/10);
		}
		for(int i =40;i <50;i++){

			width[i] = 15 - int((i-40)*5/10);
		}
		for(int i=50 ;i<60;i++){
			width[i] = 10 - int((i-50)*5/10);
		}
	System::Init();

	//camera initialization
	k60::Ov7725 camera(getCameraConfig());
	camera.Start();

	// Initialization for MOTOR1+2
	AlternateMotor::Config M1_config;
	M1_config.id = 0;
	AlternateMotor L_motor(M1_config);
	L_motorP = &L_motor;
	L_motor.SetClockwise(false);
	AlternateMotor::Config M2_config;

	M1_config.id = 1;
	AlternateMotor R_motor(M1_config);
	R_motor.SetClockwise(false);
	R_motorP = &R_motor;

	/*LCD initialization
	 *
	 *
	 */
	St7735r lcd(getLcdConfig());
	lcdP = &lcd;
	LcdTypewriter::Config typeconfig;
	typeconfig.lcd = &lcd;
	/*
	 * LED initialization
	 */
	Led led(getLedConfig());


	LcdTypewriter writer(typeconfig);
	writerP = &writer;
	/*servo initialization
	 *
	 *
	 *
	 *
	 *
	 *
	 */
	FutabaS3010 servo(getServoConfig());
	servo.SetDegree(initial_servo);
	servoP = &servo;

//joystick and flash

		Joystick::Config joystick_config;
		joystick_config.id = 0;
		joystick_config.is_active_low = true;
		Joystick joystick(joystick_config);

		Flash::Config flash_config;
		Flash flash(flash_config);

		/*Debug_console*/

		DebugConsole console(&joystick,&lcd,&writer,10);

		//	console.PushItem(item);


			console.PushItem("speed",&speed,0.05);
			console.PushItem("speed_ratio",&speed_ratio,0.05);
			console.PushItem("curve_ratio",&curve_ratio,0.05);
			console.PushItem("roundabout_ratio",&roundabout_ratio,0.05);
			console.PushItem("double_corner",&double_corner,0.1);
			console.PushItem("KP_obstacle",&KP_obstacle,0.05);
			console.PushItem("KP_curve",&KP_curve,0.1);
			console.PushItem("KP_curveless",&KP_curveless,0.1);
			console.PushItem("KP",&KP,0.1);
			console.PushItem("KP_R",&KP_R,0.3);
			console.SetOffset(0);

			console.EnterDebug(">enter program<");

//			console.SetFlash(&flash);

			//Load();








	//bluetooth initialization
	JyMcuBt106::Config BluetoothConfig;
	BluetoothConfig.id = 0;
	BluetoothConfig.baud_rate = libbase::k60::Uart::Config::BaudRate::k115200;
	BluetoothConfig.rx_isr = &bluetoothListener;
	JyMcuBt106 bluetooth1(BluetoothConfig);



	/*encoder initialization
	 *
	 *
	 */	libsc::Encoder::Config Lencoder_config, Rencoder_config;
	Lencoder_config.id = 0;
	Rencoder_config.id = 1;

	DirEncoder L_Encoder(Lencoder_config);
	DirEncoder R_Encoder(Rencoder_config);

	int32_t lastTime = System::Time();
	int32_t lastTime2 = System::Time();
	int32_t sent = 0;
	Path.reserve(20);
	detectline.reserve(20);
	double tempmk = 0;

	while(true)
	{

		if( ( System::Time() - lastTime ) >= 10){
			lastTime = System::Time();
			sent++;


			switch(joystick.GetState()){
			case Joystick::State::kUp:
				lcd_read = false;
				break;
			case Joystick::State::kDown:
				lcd_read = true;
				break;
			case Joystick::State::kIdle:
				break;
			case Joystick::State::kLeft:
				break;
			case Joystick::State::kRight:
				break;
			case Joystick::State::kSelect:
				if(!lcd_read){
					programRun = true;
				}
				break;
			}


			if(programRun && !record1){
				if(!record){
					lastTime2 = System::Time();
					record= true;
				}
				if(System::Time() - lastTime2 > 3000){
					run = true;
					record1 = true;
					}
			}



			const Byte* cameraBuffer = camera.LockBuffer();
//			printCameraImage(cameraBuffer);
			camera.UnlockBuffer();
			//Sending image

			 // compress_cam();
			  //sent = 0;
//			  const Byte imageByte = 170;
//			  bluetooth1.SendBuffer(&imageByte, 1);
//			  bluetooth1.SendBuffer(imgBuffer,600);


//
			extract_cam(cameraBuffer);
//			Filter2D();
//			Print2D();

			find_edge();
			  /*Send Data*/
			  char speedChar[15] = {};
			  //				sprintf(speedChar, "%.1f,%.3f,%.4f,%.4f,%.3f,%.4f,%.4f\n", 1.0, leftPowSpeedP, leftPowSpeedI, leftPowSpeedD, rightPowSpeedP, rightPowSpeedI, rightPowSpeedD);
			  //				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, targetAng, tempTargetAng);
			  //				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, leftSpeed, leftTempTargetSpeed);
			  //				sprintf(speedChar, "%.1f,%.2f,%.2f\n", 1.0, rightSpeed, rightTempTargetSpeed);
			  //				sprintf(speedChar, "%.1f,%.2f,%.2f,%.2f,%.2f\n", 1.0, targetAng, tempTargetAng, curSpeed, targetSpeed);
			  //			  	sprintf(speedChar, "%.1f,%.3f\n", 1.0, dt);

			  if(Lcurve||Rcurve){
				  sprintf(speedChar, "%.1f, %d\n",1.0,200);
			  }
			  else if(Lcurve_less || Rcurve_less){
				  sprintf(speedChar, "%.1f, %d\n",1.0,0);
			  }
			  else if(Lcurve_mid || Rcurve_mid){
				  sprintf(speedChar, "%.1f, %d\n",1.0,100);
			  }
			  else{
				  sprintf(speedChar, "%.1f, %d\n",1.0,-200);
			  }

			      string speedStr = speedChar;
				  const Byte speedByte = 85;
				  bluetooth1.SendBuffer(&speedByte, 1);
				  bluetooth1.SendStr(speedStr);
//			if(!run && !start){
//					if(System::Time() - lastTime2 > 3000){
//					run = true;
//					start = true;
//					}
//			}



			char buf[50];
			if(!roundabout && ! crossroad && !obstacle && R_encoder_count > 2000){
				if(num_of_round %3==1 &&corner_Lexist && corner_Rexist && !roundabout){
					roundabout = true;
					int L =0;
					int R = 0;
					L = edge[L(59)].edgeposition - 1;
					R = 78 - edge[R(59)].edgeposition;
					if(L<=R){
						L_roundabout = true;
						R_roundabout = false;
					}
					else{
						R_roundabout = true;
						L_roundabout = false;
					}
				}
				else if(num_of_round ==3 &&corner_Lexist && corner_Rexist && !crossroad){
					crossroad = true;
				}
				else if(num_of_cross %4 == 2 &&corner_Lexist && corner_Rexist && !crossroad){
					crossroad = true;
				}
				else if(corner_Lexist && corner_Rexist && !entercross){
					checkround();
				}
				else if(entercross && corner_Lexist &&corner_Rexist){
					crossroad = true;
				}
				else{
					if(!L_far_corner && !R_far_corner&&!Lcurve && !Rcurve &&!Rcurve_less && !Lcurve_less ){
						if(obstacle_detect()){
							obstacle = true;
						}
					}

				}






			}
			else if(roundabout){
				if(L_roundabout){
				int R = 0;
				if(!Round_step){

					if(!sum_of_encoder){
						L_encoder_count = 0;
						sum_of_encoder = true;
					}
					if(L_encoder_count > 2500){
						sum_of_encoder = false;
						Round_step = true;
					}
				}




				if(Round_step && ! Round_step2 && corner_Lexist && L_corner.y > 48){
					Round_step2 = true;
				}


				if(Round_step2){
					if(!sum_of_encoder){
						L_encoder_count = 0;
						sum_of_encoder = true;
					}
					if(L_encoder_count > 2800){
						sum_of_encoder = false;
						R_roundabout = false;
						L_roundabout =false;
						roundabout = false;
						num_of_round++;
						Round_step = false;
						Round_step2 = false;
						L_slow = false;
						R_slow = false;
						R_far_corner = false;
						L_far_corner =false;

					}
				}




			}
				else if(R_roundabout){
					int R = 0;
					if(!Round_step){

						if(!sum_of_encoder){
							R_encoder_count = 0;
							sum_of_encoder = true;
						}
						if(R_encoder_count > 2500){
							sum_of_encoder = false;
							Round_step = true;
						}
					}




					if(Round_step && ! Round_step2 && corner_Rexist && R_corner.y > 48){
						Round_step2 = true;
					}


					if(Round_step2){
						if(!sum_of_encoder){
							R_encoder_count = 0;
							sum_of_encoder = true;
						}
						if(R_encoder_count > 2800){
							sum_of_encoder = false;

							L_roundabout = false;
							R_roundabout =false;
							roundabout = false;
							num_of_round++;
							Round_step = false;
							Round_step2 = false;
							L_slow = false;
							R_slow = false;
							R_far_corner = false;
							L_far_corner =false;

						}
					}


				}




			}

			else if(crossroad){

				if(!sum_of_encoder){
					L_encoder_count = 0;
					sum_of_encoder = true;
				}
				if(L_encoder_count>4000 ){
					sum_of_encoder = false;
					crossroad=false;
					num_of_cross++;
					Cross_step = false;
					L_slow = false;
					R_slow = false;
					R_far_corner = false;
					L_far_corner =false;
					entercross = !entercross;

				}
			}
			else if(obstacle){
				if(!sum_of_encoder){
					L_encoder_count = 0;
					sum_of_encoder = true;
				}
				if(L_encoder_count > 4800){
					obstacle = false;
					sum_of_encoder = false;
					obstacle_left = false;
					obstacle_right = false;
					num_of_obstacle++;
				}
			}






			if(!crossroad&&!roundabout&&run && num_of_round >=2 && num_of_cross  >=3){

				 if((edge[L(2)].edgeposition > 3 ||edge[R(2)].edgeposition <77 )){
					if(startline_detect()){
					 run = false;
					}
					}
			}

			if(lcd_read){
				Print2D();
				Printpath();
				Printedge();
				char buffer[20];
//				if(obstacle){
//					sprintf(buffer, "Obstacle");
//					lcdP->SetRegion(Lcd::Rect(0,110,40,40));
//					writerP->WriteString(buffer);
//				}
//				else if(crossroad){
//					sprintf(buffer, "Cross");
//					lcdP->SetRegion(Lcd::Rect(0,110,40,40));
//					writerP->WriteString(buffer);
//				}
//				else if(roundabout){
//					sprintf(buffer, "round");
//					lcdP->SetRegion(Lcd::Rect(0,110,40,40));
//					writerP->WriteString(buffer);
//				}
//				else{
//					sprintf(buffer, "general");
//					lcdP->SetRegion(Lcd::Rect(0,110,40,40));
//					writerP->WriteString(buffer);
//				}
			}
			else{
				//lcdP->Clear();
			}

			findpath();
			L_Encoder.Update();
			L_encoder = L_Encoder.GetCount();
			L_encoder_count = L_encoder_count+L_encoder;
			L_encoder_count_test += L_encoder;
			R_Encoder.Update();
			R_encoder = R_Encoder.GetCount();
			R_encoder_count = R_encoder_count-R_encoder;
			R_encoder_count_test -= R_encoder;
			turningPID();
//


//
			char buffer[30];

			if(lcd_read)
			{
			sprintf(buffer, "%d , %d",L_encoder_count,R_encoder_count);
			lcdP->SetRegion(Lcd::Rect(0,140,120,28));
			writerP->WriteString(buffer);
			}
//			float the_ratio = 0;
//			float the_ratio1 = 0;
//			 if(test_angle >=initial_servo){
//			     	 the_ratio = -0.000001714*(test_angle)*(test_angle)+0.002246*(test_angle)+0.3272;
//
//			 }
//			 else if(test_angle < initial_servo){
//			      	the_ratio1 = -0.000002857*(test_angle)*(test_angle)+0.0051036*(test_angle)-1.276;
//
//			 }
//			sprintf(buffer, "%.2f, %.2f",the_ratio,the_ratio1);
//			lcdP->SetRegion(Lcd::Rect(0,0,120,40));
//			writerP->WriteString(buffer);
//			L_motor.SetPower(200);
//			R_motor.SetPower(200);
//			tempmk += 0.04;
//			double speed1 = speed + sin(tempmk)/2;
//
//
//			if(!run){
//									L_motor_errorsum = 0;
//									R_motor_errorsum = 0;
//									motorPID(0,820);
//			}
//			else{
//				motorPID(speed1*70,initial_servo);
//			}

				if(!programRun){

				}
				else if(!run){
					L_motor_errorsum = 0;
					R_motor_errorsum = 0;
					motorPID(0,820);
				}
				else if(motor_run && L_far_corner && R_far_corner){
					motorPID(double_corner*speed*70,servoP->GetDegree());
				}
				else if(motor_run && roundabout){
					motorPID(roundabout_ratio*speed*70,servoP->GetDegree());
				}
				else if(motor_run && crossroad){
					motorPID(crossroad_ratio*speed*70,servoP->GetDegree());
				}
				else if(motor_run && (Lcurve||Rcurve)){
					L_slow = false;
					R_slow = false;
					motorPID(curve_ratio*speed*70,servoP->GetDegree());
				}
				else if(motor_run && obstacle){
					motorPID(obstacle_ratio*speed*70,servoP->GetDegree());
				}
				else if(motor_run && !L_slow && !R_slow){
					motorPID(speed*70,servoP->GetDegree());
				}
				else if(motor_run && (L_slow || R_slow|| Lcurve_less || Rcurve_less)){
					motorPID(speed_ratio*speed*70,servoP->GetDegree());
				}

				else{
					L_motor.SetPower(0);
					R_motor.SetPower(0);
				}


				if( (L_encoder < 10 || R_encoder >-10 ) &&run && L_encoder_count > 500){
					motor_run = false;
				}
				else{
					motor_run = true;
				}

//				int time = System::Time() - lastTime;
//				sprintf(buf, "%d",time);
//
//				lcdP->SetRegion(Lcd::Rect(10,60,80,80));
//				writer.WriteString(buf);


		}
	}
		return 0;
		}




void motorPID(int ideal_count,int servoangle){
	 L_ideal_count =ideal_count;
	 R_ideal_count =ideal_count;
	 float ratio = 0;
	 if(servoangle >=initial_servo +40){
	     	 ratio = increase_ratio * (0.00000008085*(servoangle)*(servoangle)-0.001442*(servoangle)+2.143);
	     	 L_ideal_count = ratio* ideal_count;
	     	 R_ideal_count =  ideal_count;
	      }
	 else if(servoangle <initial_servo - 40){
	      			ratio = increase_ratio * (-0.0000002248*(servoangle)*(servoangle)+0.001587*(servoangle)-0.1565);
	      			L_ideal_count = ideal_count;
	      			R_ideal_count = ideal_count*ratio;
	      }
    L_motor_error = (L_ideal_count - L_encoder);
    R_motor_error = (R_ideal_count + R_encoder);

	L_premotor_error = L_motor_error;
	L_motor_errorsum += L_motor_error;

	L_motor_ideal = KP_LM*(L_motor_error)+KI_LM*(L_motor_errorsum)+KD_LM*(L_motor_error-L_premotor_error);
	if(L_motor_ideal > 1000){
		L_motor_ideal = 1000;
		L_motorP->SetClockwise(false);
	}
	if(L_motor_ideal <= 0){
		L_motorP->SetClockwise(true);
		if(L_motor_ideal < -150){
			L_motor_ideal = -150;
		}
		L_encoder_count = 0;
		L_motor_ideal = -L_motor_ideal;
	}
//	if(L_motor_ideal <0){
//		L_motorP->SetClockwise(false);
//		 L_motor_ideal=150;
//	}
	R_premotor_error = R_motor_error;
	R_motor_errorsum += R_motor_error;
//	if(R_motor_errorsum > 60000){
//		R_motor_errorsum = 60000;
//	}
//	if(R_motor_errorsum < -60000){
//		R_motor_errorsum = -60000;
//	}
	R_motor_ideal = KP_RM*(R_motor_error)+KI_RM*(R_motor_errorsum)+KD_RM*(R_motor_error-R_premotor_error);
	if(R_motor_ideal >1000){
		R_motorP->SetClockwise(false);
		R_motor_ideal = 1000;
	}
//	if(R_motor_ideal <0){
//		R_motorP->SetClockwise(true);
//		 R_motor_ideal=150;
//	}

	if(R_motor_ideal <=0){
		R_motorP->SetClockwise(true);
		if(R_motor_ideal < -150){
			R_motor_ideal = -150;
		}
		R_motor_ideal = -R_motor_ideal;
	}



	L_motorP->SetPower(L_motor_ideal);
    R_motorP->SetPower(R_motor_ideal);
}
void turningPID(){
	int a = Path.size();
	double error1 = 0;
	double error2 = 0;
	double error3 = 0;
	int middle = 0;
	int average = (L_encoder + R_encoder)/2;
	float KP_mid = 0;
	for(int row=0;row< std::min(4,a);row++){
		middle = middle+Path[row].x;
	}
	for(int row =0; row < std::min(4,a); row++){
		error1 = error1-(Path[row].x - middleline);
	}
	for(int row=4; row< std::min(8,a); row++){
		error2 = error2-(Path[row].x-middleline);
	}
	for(int row=8; row< std::min(12,a); row++){
			error3 = error3-(Path[row].x- middleline);
	}




	if(Lcurve || Rcurve){
		if(average <=70){
			ServoErr = (0.4 * error1) + (0.6 * error2);
		}
		else if(average <=170){
			float error2_ratio = 0.7+(average - 70)*0.002;
			ServoErr = (1-error2_ratio) * error1 + error2_ratio*error2;

		}
		else{
			ServoErr =(error2);
		}
	}
	else if(Lcurve_mid || Rcurve_mid){
		if(average <=70){
			ServoErr = (0.6 * error1) + (0.4 * error2);
		}
		else if(average <=170){
			float error2_ratio = 0.4+(average - 70)*0.002;
			ServoErr = (1-error2_ratio) * error1 + error2_ratio*error2;

		}
		else{
			ServoErr =(error2);
		}
	}
	else if(Lcurve_less || Rcurve_less){
		if(average <=70){
			ServoErr = (0.6 * error1) + (0.4 * error2);
		}
		else if(average <=170){
			float error2_ratio = 0.4+(average - 70)*0.002;
			ServoErr = (1-error2_ratio) * error1 + error2_ratio*error2;

		}
		else{
			ServoErr =(error2);
		}
	}
	else{
	ServoErr = (0.5 * error1) + (0.5 * error2);
	}
	int out = 0;
	for(int row = 10; row < 40; row++){
		if(edge[L(row)].edgeposition<3){
			out ++;
		}
		if(edge[R(row)].edgeposition > 77){
			out ++;
		}
	}

	//change the ServoErr to fit the turning angle


	/*
	 *
	 *
	 *
	 */
	// Incremental PID(n) = PID(n-1) + kp * (e(n)-e(n-1)) +kd *(e(n)-2e(n-1)+e(n-2)) + ki * e(n)
	if(crossroad){
		ServoErr = ServoErr * 8/a;
		idealdegree = initial_servo + int(KP_cross*ServoErr +KD_cross*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else if(roundabout && !Round_step){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else if(roundabout && Round_step && !Round_step2){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else if(roundabout && Round_step2){
		idealdegree = initial_servo + int(KP_R*ServoErr +KD_R*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else if(Lcurve || Rcurve){
		idealdegree = initial_servo + int(KP_curve*ServoErr +KD_curve*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
//		if(pre_state == 0){
//			idealdegree = (3*idealdegree+pre_idealdegree)/4;
//		}
//		else if(pre_state == 1){
//			idealdegree = (idealdegree+pre_idealdegree)/2;
//		}
//
//		pre_state = 2;
	}
	else if(Lcurve_mid || Rcurve_mid){
		if(Lcurve_mid){
			KP_mid = KP_curveless + ((L_curvature1-0.045)/0.01)*(KP_curve - KP_curveless);
		}
		else {
			KP_mid = KP_curveless + ((R_curvature1-0.045)/0.01)*(KP_curve - KP_curveless);
		}
//		sprintf(speedChar, "%.1f, %.1f\n",1.0,KP_mid);
//		 string speedStr = speedChar;
//		 const Byte speedByte = 85;
//		 bluetooth1.SendBuffer(&speedByte, 1);
//		 bluetooth1.SendStr(speedStr);
		idealdegree = initial_servo + int(KP_mid*ServoErr);
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else if(Lcurve_less || Rcurve_less){
		idealdegree = initial_servo + int(KP_curveless*ServoErr +KD_curveless*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
//		if(idealdegree < initial_servo - range){
//			idealdegree = initial_servo - range;
//		}
//		if(pre_state != 1){
//			idealdegree = (idealdegree+pre_idealdegree)/2;
//		}
//
//		pre_state =1;
	}
	else if(obstacle){
		idealdegree = initial_servo + int(KP_obstacle*ServoErr +KD_obstacle*(ServoErr-ServoPreErr));
		if(idealdegree > initial_servo + range){
			idealdegree = initial_servo + range;
		}
		if(idealdegree < initial_servo - range){
			idealdegree = initial_servo - range;
		}
	}
	else{
		idealdegree = initial_servo + int(KP*ServoErr +KD*(ServoErr-ServoPreErr));
//		if(pre_state == 1){
//			idealdegree = (idealdegree+pre_idealdegree)/2;
//		}
//		else if(pre_state ==2){
//			idealdegree = (idealdegree+ 3*pre_idealdegree)/2;
//		}
//		pre_state = 0;
	}


	if(lcd_read){
		char buffer[50];
		sprintf(buffer,"%.2f %d %d",ServoErr,idealdegree, middle/4);
		lcdP->SetRegion(Lcd::Rect(0,80,128,40));
		writerP->WriteString(buffer);
	}


//}
if(out < 45){
	servoP->SetDegree(int(idealdegree));
	ServoPreErr = ServoErr;
}
else{
	idealdegree = initial_servo + int(KP_curve*ServoPreErr);
	if(idealdegree > initial_servo + range){
		idealdegree = initial_servo + range;
	}
	if(idealdegree < initial_servo - range){
		idealdegree = initial_servo - range;
	}
	servoP->SetDegree(int(idealdegree));
}


	pre_idealdegree = idealdegree;
}

void find_edge(){

//find the left edge at bottom line
	left_bottom_exist = false;
	right_bottom_exist = false;
	if(!crossroad && !roundabout&&!obstacle){

	if(!camptr[40][CAM_H-1]){
		for(int i = 40; i > 2; i--){
			//BWW found!
			if(!camptr[i][CAM_H-1] && !camptr[i-1][CAM_H-1] && camptr[i-2][CAM_H-1] ){
				edge[L(0)].edgeposition = i - 1;
				edge[L(0)].row = CAM_H-1;
				left_bottom_exist = true;
				break;
			}
		}
		if(left_bottom_exist == false){
			edge[L(0)].edgeposition = 1;
			edge[L(0)].row = CAM_H-1;
		}
		// find the right edge at bottom line.
		for(int i = 40; i < 78 ; i++){
			//WWB found!
			if(!camptr[i][59]&&!camptr[i+1][59]&&camptr[i+2][59]){
				edge[R(0)].edgeposition = i+1;
				edge[R(0)].row = CAM_H-1;
				right_bottom_exist = true;
				break;
			}
		}
		if(right_bottom_exist == false){
			edge[R(0)].edgeposition = 78;
			edge[R(0)].row = CAM_H-1;
		}
	}
	else{
		int count = 0;
		for(int i =0;i<40;i++)
		{
			if(!camptr[i][CAM_H-1]){
				count++;
			}

		}
		if(count>25){
			// find the right edge at bottom line.
			for(int i = 40; i >0 ; i--){
				//WWB found!
				if(camptr[i][59]&&!camptr[i-1][59]&&!camptr[i-2][59]){
					edge[R(0)].edgeposition = i-1;
					edge[R(0)].row = CAM_H-1;
					break;
				}
			}
			for(int i = 40; i >0 ; i--){
				//WWB found!
				if(!camptr[i][59]&&!camptr[i-1][59]&&camptr[i-2][59]){
					edge[L(0)].edgeposition = i-1;
					edge[L(0)].row = CAM_H-1;
					left_bottom_exist = true;
					break;
				}
			}
			if(!left_bottom_exist){
				edge[L(0)].edgeposition = 1;
				edge[L(0)].row = CAM_H-1;
			}

		}
		else{
			// find the right edge at bottom line.
			for(int i = 40; i <78 ; i++){
				//WWB found!
				if(camptr[i][59]&&!camptr[i+1][59]&&!camptr[i+2][59]){
					edge[L(0)].edgeposition = i-1;
					edge[L(0)].row = CAM_H-1;
					break;
				}
			}
			for(int i = 40; i <78 ; i++){
				//WWB found!
				if(!camptr[i][59]&&!camptr[i+1][59]&&camptr[i+2][59]){
					edge[R(0)].edgeposition = i-1;
					edge[R(0)].row = CAM_H-1;
					right_bottom_exist = true;
					break;
				}
			}
			if(!right_bottom_exist){
				edge[R(0)].edgeposition = 78;
				edge[R(0)].row = CAM_H-1;
			}
		}
	}






		corner_Lexist = false;
		corner_Rexist = false;
		R_far_corner = false;
		L_far_corner =false;
		bool L_stop = false;
		bool R_stop = false;
	for(int row = 1; row<60; row ++){
		bool L_up = false;
		bool R_up = false;
		L_up = camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row-1];
		//upper point is still white for left edges
		if(!L_up){

			if(edge[L(row-1)].edgeposition == 1){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row-1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}
			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row-1;

			}
			else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row+1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row;

			}
			else if(camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
				edge[L(row)].row = edge[L(row-1)].row+1;
			}
			else {
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

		}
		//upper point is black for left edges
		else{
			if(!L_stop){
				if(edge[L(row-1)].edgeposition == 78){
					L_stop = true;
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(edge[L(row-1)].edgeposition <=2 && (camptr[40][edge[L(row-1)].row-5]||camptr[40][edge[L(row-1)].row-3]) ){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
					L_stop = true;
				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row-1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row;

				}
				else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
					edge[L(row)].row = edge[L(row-1)].row+1;

				}
				else if(!camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row+1;
				}
				else{
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
			}
			else{
				edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
				edge[L(row)].row = edge[L(row-1)].row-1;
			}

		}


		//upper point is still white for right edges
		R_up = camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row-1];
		if(!R_up){
			if(edge[R(row-1)].edgeposition == 78){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row-1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row-1;

			}
			else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row+1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row;

			}
			else if(camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
				edge[R(row)].row = edge[R(row-1)].row+1;

			}
			else {
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}

		}
		//upper point is black for right edges
		else{
			if(!R_stop){
				if(edge[R(row-1)].edgeposition == 1){
					R_stop = true;
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				if(edge[R(row-1)].edgeposition == 78 && camptr[40][edge[R(row-1)].row-5]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
					R_stop = true;
				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row-1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row;

				}
				else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
					edge[R(row)].row = edge[R(row-1)].row+1;

				}
				else if(!camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row+1;
				}
				else{
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
			}
			else{
				edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
				edge[R(row)].row = edge[R(row-1)].row-1;
			}
		}











/*
 * Trigger one L
 */
		char buf[20];
       if(row >= 12 && row % 2==0 && abs(edge[L(row)].edgeposition + edge[L(row-12)].edgeposition - 2*edge[L(row-6)].edgeposition) >= 4 && edge[L(row-6)].row >= 30 && edge[L(row-6)].edgeposition > 5  ){

//			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
//			lcdP->FillColor(0xF800);//Red

    	   	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition)&&edge[L(row-6)].row >32 && !L_far_corner){
    	   		L_far_corner = true;
//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
//    	   		sprintf(buf,"L corner");
//    	   		writerP->WriteString(buf);
    	   	}
    	   	else{
//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
//    	   		sprintf(buf," No L ");
//    	   		writerP->WriteString(buf);
    	   	}


        	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition) && !corner_Lexist && edge[L(row-6)].row >= 40){

				corner_Lexist = true;
				L_far_corner = false;
				L_corner.x = edge[L(row-6)].edgeposition;
				L_corner.y = edge[L(row-6)].row;
//				lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 4,edge[L(row-6)].row -4, 9,9));
//				lcdP->FillColor(0x0000);//Black
			}
        	if(!corner_Lexist){
//    			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
//    			lcdP->FillColor(0xF800);//Red
        	}
		}

		/*
		 * Trigger one R
		 */

		if(row >= 12 && row % 2 == 0 && abs(edge[R(row)].edgeposition + edge[R(row-12)].edgeposition - 2*edge[R(row-6)].edgeposition )>= 4 && edge[R(row-6)].row >=30 && edge[R(row-6)].edgeposition < 74){


			if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && edge[R(row-6)].row >32 && !R_far_corner){
				R_far_corner = true;
//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
//    	   		sprintf(buf,"R c");
//    	   		writerP->WriteString(buf);
			}
			else{
//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
//    	   		sprintf(buf,"No R ");
//    	   		writerP->WriteString(buf);
			}
			if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && !corner_Rexist && edge[R(row-6)].row>=40){
				corner_Rexist = true;
				R_far_corner = false;
				R_corner.x = edge[R(row-6)].edgeposition;
				R_corner.y = edge[R(row-6)].row;
//				lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-4,edge[R(row-6)].row-4, 9, 9));
//				lcdP->FillColor(0xF800);//Red
			}
			if(!corner_Rexist){
//				lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-2,edge[R(row-6)].row-2, 4, 4));
//				lcdP->FillColor(0x0000);//Red
			}
		}



	}
}
	else if(crossroad){
		for(int i = 1; i <CAM_W -5; i++){
			//BWW found!
			if(camptr[i][CAM_H-1] && camptr[i+1][CAM_H-1] && !camptr[i+2][CAM_H-1] ){
				edge[L(0)].edgeposition = i+2;
				edge[L(0)].row = CAM_H-1;
				left_bottom_exist = true;
				break;
			}
		}
		if(left_bottom_exist == false){
			edge[L(0)].edgeposition = 1;
			edge[L(0)].row = CAM_H-1;
		}
		// find the right edge at bottom line.
		for(int i = CAM_W -2; i >4 ; i--){
			//WWB found!
			if(camptr[i][CAM_H-1]&&camptr[i-1][CAM_H-1]&&!camptr[i-2][CAM_H-1]){
				edge[R(0)].edgeposition = i-2;
				edge[R(0)].row = CAM_H-1;
				right_bottom_exist = true;
				break;
			}
		}
		if(right_bottom_exist == false){
			edge[R(0)].edgeposition = CAM_W -2;
			edge[R(0)].row = CAM_H-1;
		}

		int startpoint = 0;
		startpoint = (edge[R(0)].edgeposition + edge[L(0)].edgeposition)/2;
		bool left = false;
		bool right = false;
		if(edge[L(0)].edgeposition < (78 - edge[R(0)].edgeposition )){
			right = true;
		}
		else{
			left = true;
		}
		for(int row=1; row<59; row++){
			left_bottom_exist = false;
			right_bottom_exist = false;

			if(!camptr[startpoint][CAM_H-1-row] || left){
			for(int i = startpoint;i>2;i--){
				if(!camptr[i][CAM_H-1-row] && !camptr[i-1][CAM_H-1-row] && camptr[i-2][CAM_H-1-row] ){
					edge[L(row)].edgeposition = i-1;
					edge[L(row)].row = edge[L(row-1)].row -1;
					left_bottom_exist = true;
					break;
				}
			}
			}
			else{
				for(int i = startpoint;i<78;i++){
					if(camptr[i][CAM_H-1-row] && camptr[i+1][CAM_H-1-row] && !camptr[i+2][CAM_H-1-row] ){
						edge[L(row)].edgeposition = i+2;
						edge[L(row)].row = edge[L(row-1)].row -1;
						left_bottom_exist = true;
						break;
					}
				}
			}
			if(!left_bottom_exist){
				edge[L(row)].edgeposition = 1;
				edge[L(row)].row = edge[L(row-1)].row -1;
			}

			if(!camptr[startpoint][CAM_H-1-row] || right){
			for(int i = startpoint; i<CAM_W;i++){
				if(!camptr[i][CAM_H-1-row] && !camptr[i+1][CAM_H-1-row] && camptr[i+2][CAM_H-1-row] ){
					edge[R(row)].edgeposition = i+1;
					edge[R(row)].row = edge[R(row-1)].row -1;
					right_bottom_exist = true;
					break;
				}
			}
		}
			else{
				for(int i = startpoint; i>2;i--){
					if(camptr[i][CAM_H-1-row] && camptr[i-1][CAM_H-1-row] && !camptr[i-2][CAM_H-1-row] ){
						edge[R(row)].edgeposition = i-2;
						edge[R(row)].row = edge[R(row-1)].row -1;
						right_bottom_exist = true;
						break;
					}
				}

			}
			if(!right_bottom_exist){
				edge[R(row)].edgeposition = 79;
				edge[R(row)].row = edge[R(row-1)].row -1;
			}


		}
	}
	else if(roundabout){

		if(L_roundabout){
		for(int i = CAM_W/2; i >2; i--){
			//BWW found!
			if(!camptr[i][CAM_H-1] && !camptr[i-1][CAM_H-1] && camptr[i-2][CAM_H-1] ){
				edge[L(0)].edgeposition = i-1;
				edge[L(0)].row = CAM_H-1;
				left_bottom_exist = true;
				break;
			}
		}
		if(left_bottom_exist == false){
			edge[L(0)].edgeposition = 1;
			edge[L(0)].row = CAM_H-1;
		}





			corner_Lexist = false;
			corner_Rexist = false;
			L_slow = false;
			R_slow = false;
			R_far_corner = false;
			L_far_corner =false;
			bool L_stop = false;
			bool R_stop = false;
		for(int row = 1; row<CAM_H; row ++){

			bool L_up = false;
			L_up = camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row-1];
			//upper point is still white for left edges
			if(!L_up){

				if(edge[L(row-1)].edgeposition == 1){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row-1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row-1;

				}
				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row;

				}
				else if(camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row+1;
				}
				else {
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

			}
			//upper point is black for left edges
			else{
				if(!L_stop){
					if(edge[L(row-1)].edgeposition == 78){
						L_stop = true;
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
					else if(edge[L(row-1)].edgeposition <=2 && (camptr[40][edge[L(row-1)].row-5]||camptr[40][edge[L(row-1)].row-3]) ){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
						L_stop = true;
					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row-1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row;

					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row+1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row+1;

					}
					else if(!camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row+1;
					}
					else{
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
				}
				else{
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

			}


		       if(row >= 12 && row % 2==0 && abs(edge[L(row)].edgeposition + edge[L(row-12)].edgeposition - 2*edge[L(row-6)].edgeposition) >= 4 && edge[L(row-6)].row >= 30 && edge[L(row-6)].edgeposition > 5  ){

		//			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
		//			lcdP->FillColor(0xF800);//Red

		    	   	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition)&&edge[L(row-6)].row > 35 && !L_far_corner){
		    	   		L_far_corner= true;
//		    	   		lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
//		    	   		lcdP->FillColor(0xF800);//Red
		//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
		//    	   		sprintf(buf,"L corner");
		//    	   		writerP->WriteString(buf);
		    	   	}
		    	   	else{
		//    	   		lcdP->SetRegion(Lcd::Rect(0,100,100,40));
		//    	   		sprintf(buf," No L ");
		//    	   		writerP->WriteString(buf);
		    	   	}


		        	if(Lcorner(edge[L(row-6)].row,edge[L(row-6)].edgeposition) && !corner_Lexist && edge[L(row-6)].row >= 45){

						corner_Lexist = true;
						L_slow = false;
						L_corner.x = edge[L(row-6)].edgeposition;
						L_corner.y = edge[L(row-6)].row;
//						lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 4,edge[L(row-6)].row -4, 9,9));
//						lcdP->FillColor(0x0000);//Black
					}
		        	if(!corner_Lexist){
		//    			lcdP->SetRegion(Lcd::Rect(edge[L(row-6)].edgeposition - 2,edge[L(row-6)].row -2, 4,4));
		//    			lcdP->FillColor(0xF800);//Red
		        	}
				}
		}
		for(int j=0;j<60;j++){
			edge[R(j)].row = edge[L(j)].row;
			edge[R(j)].edgeposition = edge[L(j)].edgeposition + width[j];
		}





	}
		else if(R_roundabout){
			// find the right edge at bottom line.
			for(int i = CAM_W/2; i < CAM_W -2 ; i++){
				//WWB found!
				if(!camptr[i][CAM_H-1]&&!camptr[i+1][CAM_H-1]&&camptr[i+2][CAM_H-1]){
					edge[R(0)].edgeposition = i+1;
					edge[R(0)].row = CAM_H-1;
					right_bottom_exist = true;
					break;
				}
			}
			if(right_bottom_exist == false){
				edge[R(0)].edgeposition = CAM_W -2;
				edge[R(0)].row = CAM_H-1;
			}




				corner_Lexist = false;
				corner_Rexist = false;
				L_slow = false;
				R_slow = false;
				R_far_corner = false;
				L_far_corner =false;
				bool L_stop = false;
				bool R_stop = false;
			for(int row = 1; row<CAM_H; row ++){

				bool R_up = false;

				//upper point is still white for right edges
				R_up = camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row-1];
				if(!R_up){
					if(edge[R(row-1)].edgeposition == 78){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
					else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row-1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
					else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
						edge[R(row)].row = edge[R(row-1)].row-1;

					}
					else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row+1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
						edge[R(row)].row = edge[R(row-1)].row;

					}
					else if(camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
						edge[R(row)].row = edge[R(row-1)].row+1;

					}
					else {
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}

				}
				//upper point is black for right edges
				else{
					if(!R_stop){
						if(edge[R(row-1)].edgeposition == 1){
							R_stop = true;
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
							edge[R(row)].row = edge[R(row-1)].row-1;
						}
						if(edge[R(row-1)].edgeposition == 78 && camptr[40][edge[R(row-1)].row-5]){
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
							edge[R(row)].row = edge[R(row-1)].row-1;
							R_stop = true;
						}
						else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row-1]){
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
							edge[R(row)].row = edge[R(row-1)].row-1;
						}
						else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row]){
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
							edge[R(row)].row = edge[R(row-1)].row;

						}
						else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row+1]){
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
							edge[R(row)].row = edge[R(row-1)].row+1;

						}
						else if(!camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
							edge[R(row)].row = edge[R(row-1)].row+1;
						}
						else{
							edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
							edge[R(row)].row = edge[R(row-1)].row-1;
						}
					}
					else{
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
				}


				/*
				 * Trigger one R
				 */

				if(row >= 12 && row % 2 == 0 && abs(edge[R(row)].edgeposition + edge[R(row-12)].edgeposition - 2*edge[R(row-6)].edgeposition )>= 4 && edge[R(row-6)].row >=30 && edge[R(row-6)].edgeposition < 74){


					if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && edge[R(row-6)].row > 35 && !R_far_corner){
						R_far_corner = true;
//		    	   		lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition - 2,edge[R(row-6)].row -2, 4,4));
//		    	   		lcdP->FillColor(0xF800);//Red
		//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
		//    	   		sprintf(buf,"R c");
		//    	   		writerP->WriteString(buf);
					}
					else{
		//    	   		lcdP->SetRegion(Lcd::Rect(80,100,50,40));
		//    	   		sprintf(buf,"No R ");
		//    	   		writerP->WriteString(buf);
					}
					if(Rcorner(edge[R(row-6)].row,edge[R(row-6)].edgeposition) && !corner_Rexist && edge[R(row-6)].row>=45){
						corner_Rexist = true;
						 R_far_corner= false;
						R_corner.x = edge[R(row-6)].edgeposition;
						R_corner.y = edge[R(row-6)].row;
//						lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-4,edge[R(row-6)].row-4, 9, 9));
//						lcdP->FillColor(0xF800);//Red
					}
					if(!corner_Rexist){
		//				lcdP->SetRegion(Lcd::Rect(edge[R(row-6)].edgeposition-2,edge[R(row-6)].row-2, 4, 4));
		//				lcdP->FillColor(0x0000);//Red
					}
				}



			}
			for(int j=0;j<60;j++){
				edge[L(j)].row = edge[R(j)].row;
				edge[L(j)].edgeposition = edge[R(j)].edgeposition - width[j];
			}


		}
	}
	else if(obstacle){

		for(int i = 40; i > 2; i--){
			//BWW found!
			if(!camptr[i][CAM_H-1] && !camptr[i-1][CAM_H-1] && camptr[i-2][CAM_H-1] ){
				edge[L(0)].edgeposition = i - 1;
				edge[L(0)].row = CAM_H-1;
				left_bottom_exist = true;
				break;
			}
		}
		if(left_bottom_exist == false){
			edge[L(0)].edgeposition = 1;
			edge[L(0)].row = CAM_H-1;
		}
		// find the right edge at bottom line.
		for(int i = 40; i < 78 ; i++){
			//WWB found!
			if(!camptr[i][59]&&!camptr[i+1][59]&&camptr[i+2][59]){
				edge[R(0)].edgeposition = i+1;
				edge[R(0)].row = CAM_H-1;
				right_bottom_exist = true;
				break;
			}
		}
		if(right_bottom_exist == false){
			edge[R(0)].edgeposition = 78;
			edge[R(0)].row = CAM_H-1;
		}






			bool L_stop = false;
			bool R_stop = false;
		for(int row = 1; row<60; row ++){

			bool L_up = false;
			bool R_up = false;
			L_up = camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row-1];
			//upper point is still white for left edges
			if(!L_up){

				if(edge[L(row-1)].edgeposition == 1){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row-1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}
				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row-1;

				}
				else if(camptr[edge[L(row-1)].edgeposition-1][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row;

				}
				else if(camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition-1;
					edge[L(row)].row = edge[L(row-1)].row+1;
				}
				else {
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

			}
			//upper point is black for left edges
			else{
				if(!L_stop){
					if(edge[L(row-1)].edgeposition == 78){
						L_stop = true;
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
					else if(edge[L(row-1)].edgeposition <=2 && (camptr[40][edge[L(row-1)].row-5]||camptr[40][edge[L(row-1)].row-3]) ){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
						L_stop = true;
					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row-1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row;

					}
					else if(!camptr[edge[L(row-1)].edgeposition+1][edge[L(row-1)].row+1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition+1;
						edge[L(row)].row = edge[L(row-1)].row+1;

					}
					else if(!camptr[edge[L(row-1)].edgeposition][edge[L(row-1)].row+1]){
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row+1;
					}
					else{
						edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
						edge[L(row)].row = edge[L(row-1)].row-1;
					}
				}
				else{
					edge[L(row)].edgeposition = edge[L(row-1)].edgeposition;
					edge[L(row)].row = edge[L(row-1)].row-1;
				}

			}


			//upper point is still white for right edges
			R_up = camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row-1];
			if(!R_up){
				if(edge[R(row-1)].edgeposition == 78){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row-1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
				else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
					edge[R(row)].row = edge[R(row-1)].row-1;

				}
				else if(camptr[edge[R(row-1)].edgeposition+1][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
					edge[R(row)].row = edge[R(row-1)].row;

				}
				else if(camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition+1;
					edge[R(row)].row = edge[R(row-1)].row+1;

				}
				else {
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}

			}
			//upper point is black for right edges
			else{
				if(!R_stop){
					if(edge[R(row-1)].edgeposition == 1){
						R_stop = true;
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
					if(edge[R(row-1)].edgeposition == 78 && camptr[40][edge[R(row-1)].row-5]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
						R_stop = true;
					}
					else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row-1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
					else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
						edge[R(row)].row = edge[R(row-1)].row;

					}
					else if(!camptr[edge[R(row-1)].edgeposition-1][edge[R(row-1)].row+1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition-1;
						edge[R(row)].row = edge[R(row-1)].row+1;

					}
					else if(!camptr[edge[R(row-1)].edgeposition][edge[R(row-1)].row+1]){
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row+1;
					}
					else{
						edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
						edge[R(row)].row = edge[R(row-1)].row-1;
					}
				}
				else{
					edge[R(row)].edgeposition = edge[R(row-1)].edgeposition;
					edge[R(row)].row = edge[R(row-1)].row-1;
				}
			}



		}


	}

	L_slow = false;
	R_slow = false;
	Lcurve_less = false;
	Rcurve_less = false;
	Lcurve_mid = false;
	Rcurve_mid = false;
	Lcurve = false;
	Rcurve = false;



//
//	if(edge[L(55)].edgeposition - edge[L(45)].edgeposition <-6&&edge[L(55)].row - edge[L(45)].row > -3 && edge[R(59)].row - edge[R(50)].row >-3 && edge[R(59)].edgeposition - edge[R(50)].edgeposition >6&&edge[L(45)].row > 20){
//		L_slow = true;
//	}
//	else if(edge[R(55)].edgeposition - edge[R(45)].edgeposition >6&&edge[R(55)].row - edge[R(45)].row >  -3 && edge[L(59)].row - edge[L(59)].row > -3&& edge[L(59)].edgeposition - edge[L(59)].edgeposition > 6&&edge[R(45)].row > 20){
//		R_slow = true;
//	}


	for(int x=0;x<15;x++){
		Left_x[x] = -1;
		Right_x[x] = -1;
	}

	for(int i=0; i< 60 ;i++){
		/*
			* Left!
		*/
		if(edge[L(i)].row == 55 && Left_x[0] == -1 ){
			Left_x[0] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 52 && Left_x[1] == -1){
			Left_x[1] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 49 && Left_x[2] == -1){
			Left_x[2] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 46 && Left_x[3] == -1){
			Left_x[3] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 43 && Left_x[4] == -1){
			Left_x[4] = edge[L(i)].edgeposition;
			continue;

		}
		if(edge[L(i)].row == 40 && Left_x[5] == -1){
			Left_x[5] = edge[L(i)].edgeposition;
			continue;
	 	}
		if(edge[L(i)].row == 37 && Left_x[6] == -1){
			Left_x[6] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 34 && Left_x[7] == -1){
			Left_x[7] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 31 && Left_x[8] == -1){
			Left_x[8] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 28 && Left_x[9] == -1){
			Left_x[9] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 25 && Left_x[10] == -1){
			Left_x[10] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 22 && Left_x[11] == -1){
			Left_x[11] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 19 && Left_x[12] == -1){
			Left_x[12] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 16 && Left_x[13] == -1){
			Left_x[13] = edge[L(i)].edgeposition;
			continue;
		}
	}
				/*
				 * Right
				 */
			for(int i=0; i< 60 ;i++){
				if(edge[R(i)].row ==55 && Right_x[0] == -1){
					Right_x[0] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==52 && Right_x[1] == -1){
					Right_x[1] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==49 && Right_x[2] == -1){
					Right_x[2] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==46 && Right_x[3] == -1){
					Right_x[3] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==43 && Right_x[4] == -1){
					Right_x[4] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==40 && Right_x[5] == -1){
					Right_x[5] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==37 && Right_x[6] == -1){
					Right_x[6] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==34 && Right_x[7] == -1){
					Right_x[7] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==31 && Right_x[8] == -1){
					Right_x[8] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==28 && Right_x[9] == -1){
					Right_x[9] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==25 && Right_x[10] == -1){
					Right_x[10] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==22 && Right_x[11] == -1){
					Right_x[11] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==19 && Right_x[12] == -1){
					Right_x[12] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==16 && Right_x[13] == -1){
					Right_x[13] = edge[R(i)].edgeposition;
					continue;
				}


			}
	char buffer[50];



if(!obstacle && !crossroad &&!roundabout){




	L_curvature1 =	curvature(edge[L(1)],edge[L(15)],edge[L(30)]);
	L_curvature2 =	curvature(edge[L(1)],edge[L(30)],edge[L(59)]);
	R_curvature1 =	curvature(edge[R(1)],edge[R(15)],edge[R(30)]);
	R_curvature2 = 	curvature(edge[R(1)],edge[R(30)],edge[R(59)]);

if( Right_x[4] > CAM_W -4 &&Right_x[5] > CAM_W -4 && Right_x[6] > CAM_W -4 &&!L_far_corner && !corner_Lexist){
	Rcurve = true;
	Lcurve = false;

}
else if(Left_x[4] <4 && Left_x[5] <4 &&Left_x[6] <4 &&  !R_far_corner && !corner_Rexist){
	Lcurve = true;
	Rcurve = false;
}
else if(L_curvature1 > 0.055 &&  !(R_far_corner && L_far_corner)){
	Lcurve = true;
}
else if(R_curvature1 > 0.055 &&  !(R_far_corner && L_far_corner)){
	Rcurve = true;
}

else if(L_curvature1 >= 0.045 && L_curvature1 <= 0.055 &&  !(R_far_corner && L_far_corner)){
	Lcurve_mid = true;
}
else if(R_curvature1 >= 0.045 && R_curvature1 <= 0.055 &&  !(R_far_corner && L_far_corner)){
	Rcurve_mid = true;
}
else if(L_curvature1 >= 0.025 && L_curvature1 <= 0.045 &&  !(R_far_corner && L_far_corner)){
	Lcurve_less = true;
}
else if(R_curvature1 >= 0.025 && R_curvature1 <= 0.045 &&  !(R_far_corner && L_far_corner)){
	Rcurve_less = true;
}
else if(Left_x[5] - Left_x[0] >10 ){
		Rcurve_less = true;
}
else if(Right_x[5] - Right_x[0] < -10){
	Lcurve_less = true;
}
if(L_curvature2 >0.05){
	L_slow = true;
}
else if(R_curvature2 > 0.05){
	R_slow = true;
}
}



if(lcd_read){

//float L_angle1 =0;
//float L_angle2 =0;
//float R_angle1 =0;
//float R_angle2 =0;
//
//
//L_angle1 = angle(edge[L(1)],edge[L(15)],edge[L(30)]);
//L_angle2 = angle(edge[L(1)],edge[L(30)],edge[L(59)]);
//R_angle1 =	angle(edge[R(1)],edge[R(15)],edge[R(30)]);
//R_angle2 =	angle(edge[R(1)],edge[R(30)],edge[R(59)]);
//	sprintf(buffer, "%.4f, %.4f",L_angle1,L_angle2);
//	lcdP->SetRegion(Lcd::Rect(0,100,128,40));
//	writerP->WriteString(buffer);
//
//
//	sprintf(buffer, "%.4f, %.4f",R_angle1,R_angle2);
//	lcdP->SetRegion(Lcd::Rect(0,120,128,40));
//	writerP->WriteString(buffer);



	sprintf(buffer, "%.4f, %.4f",L_curvature1,L_curvature2);
	lcdP->SetRegion(Lcd::Rect(0,100,128,40));
	writerP->WriteString(buffer);


	sprintf(buffer, "%.4f, %.4f",R_curvature1,R_curvature2);
	lcdP->SetRegion(Lcd::Rect(0,120,128,40));
	writerP->WriteString(buffer);

//	sprintf(buffer, "%d, %d",Left_x[4] - Left_x[0],Left_x[8] - Left_x[0]);
//	lcdP->SetRegion(Lcd::Rect(80,60,40,40));
//	writerP->WriteString(buffer);
//
//
//	sprintf(buffer, "%d, %d",Right_x[4] - Right_x[0],Right_x[8] - Right_x[0]);
//	lcdP->SetRegion(Lcd::Rect(80,80,40,40));
//	writerP->WriteString(buffer);





	if(Rcurve){
		sprintf(buffer, "R");
		lcdP->SetRegion(Lcd::Rect(80,0,40,40));
		writerP->WriteString(buffer);
	}
	else if(Lcurve){
		sprintf(buffer, "L");
		lcdP->SetRegion(Lcd::Rect(80,0,40,40));
		writerP->WriteString(buffer);
	}
	else if(Lcurve_less){
		sprintf(buffer, "L_s");
		lcdP->SetRegion(Lcd::Rect(80,0,40,40));
		writerP->WriteString(buffer);
	}
	else if(Rcurve_less){
		sprintf(buffer, "R_s");
		lcdP->SetRegion(Lcd::Rect(80,0,40,40));
		writerP->WriteString(buffer);
	}

	else{
		sprintf(buffer, "Normal");
		lcdP->SetRegion(Lcd::Rect(80,0,40,40));
		writerP->WriteString(buffer);
	}

	if(L_slow)
	{
		sprintf(buffer, "L_slow");
		lcdP->SetRegion(Lcd::Rect(80,20,40,40));
		writerP->WriteString(buffer);
	}
	else if(R_slow)
	{
		sprintf(buffer, "R_slow");
		lcdP->SetRegion(Lcd::Rect(80,20,40,40));
		writerP->WriteString(buffer);
	}
	else{
		sprintf(buffer, "no_slow");
		lcdP->SetRegion(Lcd::Rect(80,20,40,40));
		writerP->WriteString(buffer);
	}
}

}

void findpath(){

	Path.clear();
		if(!roundabout && !crossroad &&!obstacle){
			/*
			 * Path
			 */
			int time = 0;
			for(int row = 0; row< 14;row++){
				if(Left_x[row] != -1 && Right_x[row] != -1){
					if(Lcurve || Lcurve_less || Lcurve_mid){
						if(Right_x[row] > 2){
							if(Left_x[1] <= 5){
								coor temp;
								//temp.x = Right_x[row] - offset;
								temp.x = Right_x[row] - width[5+row*3]/2;
								temp.y = 58-3*(row+1);
								Path.push_back(temp);
							}
							else{
								//if(Left_x[row] > 5){
									coor temp;
									temp.x = ((time/5.0+1)*Left_x[row] + Right_x[row])/(time/5.0+2);
									temp.y = 58-3*(row+1);
									time++;
									Path.push_back(temp);
//								}
//								else{
//									coor temp;
//									//temp.x = Right_x[row] - offset;
//									temp.x = Right_x[row] - width[5+row*3];
//									temp.y = 58-3*(row+1);
//									Path.push_back(temp);
//									}
								}
							}
					}
					else if(Rcurve || Rcurve_less ||Rcurve_mid){

						if(Left_x[row] < 75){
							if(Right_x[1] >=75){
								coor temp;
								temp.x = Left_x[row] + width[5+row*3]/2;
								//temp.x = Left_x[row] + offset + 2*row +4;
								temp.y = 58-3*(row+1);
								Path.push_back(temp);
							}
							else{
								//if(Right_x[row] < 75){
									coor temp;
									temp.x = ((time/5.0+1)*Right_x[row] + Left_x[row])/(time/5.0+2);
									temp.y = 58-3*(row+1);
									Path.push_back(temp);
									time++;

								//}

//								else{
//									coor temp;
//									//temp.x = Path[Path.size()-1].x + 2;
//									temp.x = Left_x[row] + width[5+row*3];
//									temp.y = 58-3*(row+1);
//									Path.push_back(temp);
//								}
						}
						}
					}

					else if(Left_x[row] != 1 && Left_x[row] != 78 && Right_x[row] != 78 && Right_x[row] != 1){
					coor temp;
					temp.x = (Left_x[row] + Right_x[row])/2;
					temp.y = 58-3*(row+1);
					Path.push_back(temp);
					}

					else if(Left_x[row] == 1 && Right_x[row] != 78){
						coor temp;
						temp.x = Right_x[row] - offset - row;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

					else if(Left_x[row] != 1 && Right_x[row] == 78){
						coor temp;
						temp.x = Left_x[row] + offset + row;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

					else{
						coor temp;
						temp.x = (Left_x[row] + Right_x[row])/2;
						temp.y = 58-3*(row+1);
						Path.push_back(temp);
					}

				}



			}


		}
		else if(roundabout){
			for(int row=0; row<14;row++){

					coor temp;
					temp.x = (Left_x[row] + Right_x[row])/2;
					temp.y = 58-3*(row+1);
					Path.push_back(temp);




			}

		}
		else if(crossroad){

			for(int i =0;i<14;i++){
							if(Left_x[i] >10 && Right_x[i] < 70){
								coor temp;
								temp.x = (Left_x[i] + Right_x[i])/2;
								temp.y = 58-3*(i+1);
								Path.push_back(temp);
							}
						}


	}
		else if(obstacle){
			if(obstacle_left){
					for(int i =0;i<14;i++){

							coor temp;
							temp.x = Right_x[i] - width[CAM_H-(55-i*3)]*2.5/10;
							temp.y = 58-3*(i+1);
							Path.push_back(temp);

										}
			}
			else if(obstacle_right){
				for(int i =0;i<14;i++){

							coor temp;
							temp.x = Left_x[i] + width[CAM_H-(55-i*3)]*2.8/10;
							temp.y = 58-3*(i+1);
							Path.push_back(temp);

							}
			}
//			for(int i =0;i<14;i++){
//
//								coor temp;
//								temp.x = (Left_x[i] + Right_x[i])/2;
//								temp.y = 58-3*(i+1);
//								Path.push_back(temp);
//
//						}
		}

}
bool Lcorner(int i,int j){
	double corner_count = 0.0 ;// How many black pixel
	double corner_num = 81 ;// How many pixel count
	double corner_ratio = 0.0 ;//Percentage of area of black pixel
	// 9x9 Box
	for(int n=i-4;n <= i+4;n++){
		for(int m=j-4;m <= j+4;m++){
			// 80*60
			if(camptr[m][n]){
				corner_count++;
			}
		}
	}
	corner_ratio = corner_count / corner_num;
//	char buffer[50];
//	sprintf(buffer, "%.2f",corner_count);
//	lcdP->SetRegion(Lcd::Rect(80,90,40,40));
//	writerP->WriteString(buffer);
	// If the percentage of black pixel is 20 - 30%, then it is corner.
	if(corner_ratio >= 0.2 && corner_ratio <= 0.35){
		return true;
		}

return false;
	}

bool roundabout_detect(int i,int j){
	int black_count = 0;
	for(int n = i-10; n<i;n++){
		for(int m = j-5; m<j+5;m++){
			if(camptr[m][n]){
				black_count++;}
			}
	}
//	lcdP->SetRegion(Lcd::Rect(j-5,i-10,10,10));
//	lcdP->FillColor(0x0000);
	if(black_count > 20){
		return true;
	}
	return false;

}




void checkround(){

	int middle_x = 0;
	int middle_y = 0;
	int bottom_x = 0;
	int bottom_y = 0;
	double k =0;
	middle_x = (L_corner.x + R_corner.x)/2;
	middle_y = (L_corner.y + R_corner.y)/2;
	bottom_x = (edge[L(1)].edgeposition + edge[R(1)].edgeposition)/2;
	bottom_y = (edge[L(1)].row + edge[R(1)].row)/2;
	k = double(bottom_y - middle_y)/double(middle_x - bottom_x);
	detectline.clear();
	if(k<3 && k>0){
		k=3;
	}
	if(k>-3 && k <0){
		k = -3;
	}
	for(int row =0; row< (middle_y); row++){
		coor temp;
		temp.x = round((1*(row+1)/k) + middle_x);
		temp.y = middle_y - (row+1);
		detectline.push_back(temp);
	}
	int first_black_y = 0;
	int second_black_y = 0;
	int first_black_x = 0;
	int second_black_x = 0;
	bool first = false;
	bool second = false;
	for(int row=0;row<detectline.size() -3;row++){
//		lcdP->SetRegion(Lcd::Rect(detectline[row].x,detectline[row].y, 1, 1));
//		lcdP->FillColor(0xF800);//red
		if(!first && !camptr[round(detectline[row].x)][round(detectline[row].y)] &&!camptr[round(detectline[row+1].x)][round(detectline[row+1].y)] &&camptr[round(detectline[row+2].x)][round(detectline[row+2].y])){
			first = true;
			first_black_y = detectline[row+2].y;
			first_black_x = detectline[row+2].x;
		}
		else if(first &&!second && camptr[round(detectline[row].x)][round(detectline[row].y)] &&camptr[round(detectline[row+1].x)][round(detectline[row+1].y)] && !camptr[round(detectline[row+2].x)][round(detectline[row+2].y])){
			second = true;
			second_black_y = detectline[row+1].y;
			second_black_x = detectline[row+1].x;
			break;
		}
	}




	if(first && second){
		int f =-1;
		int s =-1;
		int x = (first_black_x + second_black_x)/2;
		int y = (first_black_y + second_black_y)/2;
		for(int i =x;i<57;i++){
			if(camptr[i][y] && camptr[i+1][y]&&!camptr[i+2][y] && !camptr[i+3][y]){
				f = i+2;
				break;
			}
		}
		for(int i =x;i > 0;i--){
			if(camptr[i][y] && camptr[i-1][y]&& !camptr[i-2][y]&& !camptr[i-3][y]){
				s = i-2;
				break;
			}
		}

		if(abs(f+s - L_corner.x-R_corner.x)<10){
			roundabout = true;
				int L =0;
				int R = 0;
			L = edge[L(59)].edgeposition - 1;
			R = 78 - edge[R(59)].edgeposition;
			if(L<=R){
				L_roundabout = true;
				R_roundabout = false;
			}
			else{
				R_roundabout = true;
				L_roundabout = false;
			}




		}
		else{
			crossroad = true;
		}

	}
	else{
		crossroad = true;
	}
//	if(!roundabout){
//		crossroad = true;
//	}
//	else{

//
//	}



}


bool Rcorner(int i,int j){
	double corner_count = 0.0 ;// How many black pixel
	double corner_num =81 ;// How many pixel count
	double corner_ratio = 0.0 ;//Percentage of area of black pixel
	// 9x9 Box
	for(int n=i-4;n <= i+4;n++){
		for(int m=j-4;m <= j+4;m++){
			// 80*60
			if(camptr[m][n]){
				corner_count++;
			}
		}
	}


	corner_ratio = corner_count / corner_num;
//	char buffer[50];
//	sprintf(buffer, "%.2f",corner_count);
//	lcdP->SetRegion(Lcd::Rect(80,110,40,40));
//	writerP->WriteString(buffer);
	// If the percentage of black pixel is 20 - 30%, then it is corner.
	if(corner_ratio >= 0.2 && corner_ratio <= 0.35){
		return true;
		}

return false;
	}



void Printedge(){
	for(int j = 0; j< 60 ;j ++){

		if(j%5 ==0){
			lcdP->SetRegion(Lcd::Rect(edge[L(j)].edgeposition, edge[L(j)].row, 2, 2));
			lcdP->FillColor(0x0000);//black
		}
		else{
			lcdP->SetRegion(Lcd::Rect(edge[L(j)].edgeposition, edge[L(j)].row, 2, 2));
			lcdP->FillColor(0xF800);//red
		}

	}

	for(int j=0;j<60;j++){



		if(j%5 ==0){
				lcdP->SetRegion(Lcd::Rect(edge[R(j)].edgeposition, edge[R(j)].row, 2, 2));
				lcdP->FillColor(0XF800);//black
		}
		else{
			lcdP->SetRegion(Lcd::Rect(edge[R(j)].edgeposition, edge[R(j)].row, 2, 2));
			lcdP->FillColor(0X0000);//black
		}
	}
}
void Printpath(){
	for(int i = 0; i < Path.size(); i++){
		if(Path[i].x >= 0){
		lcdP->SetRegion(Lcd::Rect(round(Path[i].x),Path[i].y, 2, 2));
		lcdP->FillColor(0xF800);//red
	}
	}
	char buffer[50];
	sprintf(buffer," %d ",Path.size());
	lcdP->SetRegion(Lcd::Rect(80,40,40,40));
	writerP->WriteString(buffer);
}
void printCameraImage(const Byte* image)
{
	lcdP->SetRegion(Lcd::Rect(0, 0, CAM_W, cam_h));
	lcdP->FillBits(0x001F, 0xFFFF, image, CAM_W * cam_h);
}
bool startline_detect(){

	int Left_x[9];
	int Right_x[9];
	for(int x=0;x<9;x++){
		Left_x[x] = -1;
		Right_x[x] = -1;
	}
	for(int i=0; i< 60 ;i++){
		/*
			* Left!
		*/
		if(edge[L(i)].row == 58 && Left_x[0] == -1 ){
			Left_x[0] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 57 && Left_x[1] == -1){
			Left_x[1] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 56 && Left_x[2] == -1){
			Left_x[2] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 55 && Left_x[3] == -1){
			Left_x[3] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 54 && Left_x[4] == -1){
			Left_x[4] = edge[L(i)].edgeposition;
			continue;

		}
		if(edge[L(i)].row == 53 && Left_x[5] == -1){
			Left_x[5] = edge[L(i)].edgeposition;
			continue;
	 	}
		if(edge[L(i)].row == 52 && Left_x[6] == -1){
			Left_x[6] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 51 && Left_x[7] == -1){
			Left_x[7] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 50 && Left_x[8] == -1){
			Left_x[8] = edge[L(i)].edgeposition;
			continue;
		}
	}
				/*
				 * Right
				 */
			for(int i=0; i< 60 ;i++){
				if(edge[R(i)].row ==58 && Right_x[0] == -1){
					Right_x[0] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==57 && Right_x[1] == -1){
					Right_x[1] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==56 && Right_x[2] == -1){
					Right_x[2] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==55 && Right_x[3] == -1){
					Right_x[3] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==54 && Right_x[4] == -1){
					Right_x[4] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==53 && Right_x[5] == -1){
					Right_x[5] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==52 && Right_x[6] == -1){
					Right_x[6] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==51 && Right_x[7] == -1){
					Right_x[7] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==50 && Right_x[8] == -1){
					Right_x[8] = edge[R(i)].edgeposition;
					continue;
				}

			}



	int L = 0;
	int R = 0;
	int time = 0;
	for(int row = 8; row >=0; row--){
		L = Left_x[row];
		R = Right_x[row];
		bool reverse = false;
		time =0;
		for(int count = L; count<R-2;count++){
			if(!camptr[count][50+(8-row)] &&  camptr[count+1][50+(8-row)]&&!reverse){
				time++;
				reverse = true;
			}
			else if(reverse){
				if(camptr[count][50+(8-row)] && !camptr[count+1][50+(8-row)]){
					time++;
					reverse= false;
				}
			}


		}
		if(time > 8){
			return true;
		}
	}

	return false;

}
bool obstacle_detect(){

	int Left_x[6];
	int Right_x[6];
	for(int x=0;x<6;x++){
		Left_x[x] = -1;
		Right_x[x] = -1;
	}
	for(int i=0; i< 60 ;i++){
		/*
			* Left!
		*/
		if(edge[L(i)].row == 35 && Left_x[0] == -1 ){
			Left_x[0] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 33 && Left_x[1] == -1){
			Left_x[1] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 31 && Left_x[2] == -1){
			Left_x[2] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 29 && Left_x[3] == -1){
			Left_x[3] = edge[L(i)].edgeposition;
			continue;
		}
		if(edge[L(i)].row == 27 && Left_x[4] == -1){
			Left_x[4] = edge[L(i)].edgeposition;
			continue;

		}
		if(edge[L(i)].row == 25 && Left_x[5] == -1){
			Left_x[5] = edge[L(i)].edgeposition;
			continue;
	 	}
	}
				/*
				 * Right
				 */
			for(int i=0; i< 60 ;i++){
				if(edge[R(i)].row ==35 && Right_x[0] == -1){
					Right_x[0] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==33 && Right_x[1] == -1){
					Right_x[1] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==31 && Right_x[2] == -1){
					Right_x[2] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==29 && Right_x[3] == -1){
					Right_x[3] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==27 && Right_x[4] == -1){
					Right_x[4] = edge[R(i)].edgeposition;
					continue;
				}
				if(edge[R(i)].row ==25 && Right_x[5] == -1){
					Right_x[5] = edge[R(i)].edgeposition;
					continue;
				}


			}

	int L = 0;
	int R = 0;
	int time = 0;
	int first = 0;
	int second = 0;
	int count = 0;
	int left = 0;
	int right = 0;


	for(int row = 5; row>=0;row--){
		if(Right_x[row] > CAM_W -4 || Right_x[row]==-1){
			return false;
		}
		if(Left_x[row] < 4|| Left_x[row] ==-1){
			return false;
		}

	}



	for(int row = 5; row >=0; row--){
		L = Left_x[row];
		R = Right_x[row];
		bool reverse = false;
		time =0;
		for(count = L; count<=R-2;count++){
			if(!reverse && !camptr[count][25+2*(5-row)] &&  camptr[count+1][25+2*(5-row)]&& camptr[count+2][25+2*(5-row)]){
				first = count+1-L;
				reverse =true;
				time++;
			}
			else if(reverse){
				if(camptr[count][25+2*(5-row)] && camptr[count+1][25+2*(5-row)] && !camptr[count+2][25+2*(5-row)]){
					time++;
					reverse= false;
					second = count + 1-L-first;
				}
			}

		}
			if(time == 2 && second >=3 && second <=6){
				if(first >= 4){
						obstacle_right = true;
					}
					else{
						obstacle_left = true;
					}
					return true;
			}
	}

	return false;
}

void reset(){
	int	num_of_round = 0;
	int num_of_cross = 0;
	bool corner_Lexist = false;
	bool corner_Rexist = false;
	bool L_slow = false;
	bool R_slow = false;
	bool crossroad = false;
	bool entercross =false;
	bool Rcross = false;
	bool roundabout = false;
	bool L_roundabout = false;
	bool R_roundabout = false;
	bool Round_step = false;
	bool Round_step2 = false;
	bool Cross_step = false;
	bool Lcurve = false;
	bool Rcurve = false;
	bool sum_of_encoder = false;
}

float curvature(set point1,set point2,set point3){
	int x1 = point1.edgeposition;
	int y1 = CAM_H - point1.row;
	int x2 = point2.edgeposition;
	int y2 = CAM_H - point2.row;
	int x3 = point3.edgeposition;
	int y3 = CAM_H - point3.row;

	int area = abs((x2-x1)*(y3-y1) - (x3-x1)*(y2-y1))/2;
	int length1 =0;
	int length2 =0;
	int length3 =0;
	length1 = func((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	length2 = func((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));
	length3 = func((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));
	float ans = float(4*area)/((length1)*(length2)*(length3));
	return ans;
}

double angle(set point1,set point2,set point3){
	int x1 = point1.edgeposition;
	int y1 = CAM_H - point1.row;
	int x2 = point2.edgeposition;
	int y2 = CAM_H - point2.row;
	int x3 = point3.edgeposition;
	int y3 = CAM_H - point3.row;
	int length1 =0;
	int length2 =0;
	int length3 =0;
	length1 = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	length2 = sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));
	length3 = sqrt((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));

	double cos_value = double(-length3*length3 + length1*length1+length2*length2)/(2*(length2*length1));


	if(cos_value >=1 || cos_value < -1){
		return 0;
	}


	double ans = acos(cos_value) * 180 /3.1416;
	return ans;
}


int func(int x){
	unsigned int ans=0,p=0X80;
	while(p!=0){
		ans+=p;
		if(ans*ans>x){
			ans-=p;
		}
		p =(p/2);
	}
	return ans;
}
