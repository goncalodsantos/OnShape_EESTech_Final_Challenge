/**
 * Torque control example of adaptive gripper. Based on SimpleFOC library.
 *
 * 1. After flashing this code to the XMC4700 Relax Kit, angle alignment will be
 *    applied. The gripper must be opened to its fullest extent to release the gear
 *    and minimize load for alignment.
 *
 * 2. After angle alignment, attach the gears (manually close the gripper a bit
 *    to align the gears) and you can start controlling the gripper's opening and
 *    closing. Pressing button 1 on the board will close the gripper, and pressing
 *    button 2 will open it. Note: There is no upper limit set for opening, so it
 *    is possible that the gears may detach if the maximum is exceeded.
 *
 * 3. Open the serial monitor/serial plotter to view data from the magnetic
 *    sensors placed under the TPU material on top of the clip. When the gripping
 *    clip grabs an object and generates pressure, the data changes.
 *
 * This is a basic example; you can be creative to improve this gripper!
 */

/////////////////////////////// GENERAL CONFIGURATION /////////////////////////////////

#include "TLE5012Sensor.h"
#include "TLx493D_inc.hpp"
#include "config.h"
#include <SimpleFOC.h>

// define SPI pins for TLE5012 sensor - Communication PINS       
#define PIN_SPI1_SS0 94  // Chip Select (CS) pin            
#define PIN_SPI1_MOSI 69 // MOSI pin
#define PIN_SPI1_MISO 95 // MISO pin
#define PIN_SPI1_SCK 68  // SCK pin

// create an instance of SPIClass3W for 3-wire SPI communication
tle5012::SPIClass3W tle5012::SPI3W1(2);
// create an instance of TLE5012Sensor
TLE5012Sensor tle5012Sensor(&SPI3W1, PIN_SPI1_SS0, PIN_SPI1_MISO, PIN_SPI1_MOSI,
                            PIN_SPI1_SCK);

// BLDC motor instance BLDCMotor (polepairs, motor phase resistance, motor KV
// rating, motor phase inductance)
BLDCMotor motor = BLDCMotor(
    7, 0.24, 360,
    0.000133); // 7 pole pairs, 0.24 Ohm phase resistance, 360 KV and 0.000133H
// you can find more data of motor in the doc

// define driver pins
const int U = 11;
const int V = 10;
const int W = 9;
const int EN_U = 6;
const int EN_V = 5;
const int EN_W = 3;

// BLDC driver instance
BLDCDriver3PWM driver = BLDCDriver3PWM(U, V, W, EN_U, EN_V, EN_W);

#if ENABLE_MAGNETIC_SENSOR
// create a instance of 3D magnetic sensor       
using namespace ifx::tlx493d;
TLx493D_A2B6 dut(Wire1, TLx493D_IIC_ADDR_A0_e);
// define the number of calibration samples
const int CALIBRATION_SAMPLES = 20;
// offsets for calibration
double xOffset = 0, yOffset = 0, zOffset = 0;
#endif


///////////////////////////////SERIAL COM///////////////////////////////

char buffer[64];
int idx = 0;
char ultimo_comando[2] = "s";  // 's' de stop


///////////////////////////////MOTOR///////////////////////////////

// voltage set point variable
float target_angle = 0;
float step_angle = 0.15;                               // step size for angle adjustment

float speed = 0;
float speed_P = 50, speed_I = 0, speed_D = 0;


volatile bool flag_stop_motor = false; 

volatile unsigned long previousMillis_mot = 0;
volatile unsigned long interval_mot = 0;

///////////////////////////////MAGNETIC SENSOR //////////////////////////////////////////////

unsigned long previousMillis_mag = 0;                   //last time magnetic sensor was read
int interval_mag = 25;                                  //interval between magnetic sensor readings

volatile float MinGrip = 0.5;
volatile float MaxGrip = 1.1;
volatile float Squish = 0.2;

double mag_x, mag_y, mag_z, prevmagZ;

volatile unsigned long previousMillis_grip = 0;         //grip check time
volatile int interval_grip = 30;                        //grip check interval

/////////////////////////////// DEBUG ///////////////////////////////
int DEBUG = 0;

/////////////////////////////// Código Principal ///////////////////////////////
void setup() {
  // use monitoring with serial
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  //SimpleFOCDebug::enable(&Serial);

  // initialise magnetic sensor hardware
  tle5012Sensor.init();
  
  // link the motor to the sensor
  motor.linkSensor(&tle5012Sensor);

  // power supply voltage
  driver.voltage_power_supply = 12;
  // limit the maximal dc voltage the driver can set
  // as a protection measure for the low-resistance motors
  // this value is fixed on startup
  driver.voltage_limit = 6;
  
  if (!driver.init()) {
    Serial.println("Driver init failed!");
    return;
  }
  
  // link the motor and the driver
  motor.linkDriver(&driver);

  // aligning voltage
  motor.voltage_sensor_align = 4;
  
  // choose FOC modulation (optional)
  //motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  // set motion control loop to be used
  motor.controller = MotionControlType::velocity_openloop;

  
  //ANGULAR CONTROL
  // velocity PI controller parameters
  // default P=0.5 I = 10
  motor.PID_velocity.P = 2;
  motor.PID_velocity.I = 0.02;
  motor.PID_velocity.D = 0.0;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  motor.PID_velocity.output_ramp = 1000; // limita a variação do setpoint

  //default voltage_power_supply
  motor.voltage_limit = 6;

  // maximal velocity of the position control
  // default 20
  // Mesmo que o erro de posição seja grande (por exemplo, ângulo atual = 0° e target_angle = 180°), o motor nunca vai girar mais rápido que 4 rad/s para alcançar o destino.
  motor.velocity_limit = 10; // limita a velocidade máxima angular do motor quando ele está a tentar atingir um certo ângulo

  // velocity low pass filtering
  // default 5ms - try different values to see what is the best. 
  // the lower the less filtered
  //motor.LPF_velocity.Tf = 0.01;

  // angle P controller 
  motor.P_angle.P = 20;   // default P=20
  motor.P_angle.I = 0;  
  motor.P_angle.D = 0;

  // comment out if not needed
  // motor.useMonitoring(Serial);

  // initialize motor
  motor.init();
  // align sensor and start FOC
  motor.initFOC();
  Serial.println(F("Motor ready."));

  // start 3D magnetic sensor
  dut.begin();
  
  // calibrate 3D magnetic sensor to get the offsets
  calibrateSensor();
  Serial.println("3D magnetic sensor Calibration completed.");

  // set the pin modes for buttons
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);

  Serial.print("setup done.\n");
  _delay(1000);
}

/////////////////////////////// LOOP PRINCIPAL ///////////////////////////////
void loop() {
  // LOOP COM BOTÕES

  /*
  motor.loopFOC();

  check_buttons();

  get_magnetic_readings();

  control_vel_loop();

  check_pressure();
  
  motor_loop();*/


 // LOOP COM SERIAL
  motor.loopFOC();
  
  check_serial_gui();
  
  get_magnetic_readings();

  last_command();
  
  control_vel_loop();
 
  check_pressure();
  
  motor_loop();

  if(DEBUG) 
  {
    Serial.println("");
    Serial.println("------------------------------ END OF LOOP ------------------------");
    Serial.println("");
  }
  
}

/////////////////////////////// FUNÇÕES ///////////////////////////////

void calibrateSensor() 
{  
  double sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < CALIBRATION_SAMPLES; ++i) {
    double temp;
    double valX, valY, valZ;

    dut.getMagneticFieldAndTemperature(&valX, &valY, &valZ, &temp);
    sumX += valX;
    sumY += valY;
    sumZ += valZ;

    delay(100); // Adjust delay as needed
  }

  // Calculate average offsets
  xOffset = sumX / CALIBRATION_SAMPLES;
  yOffset = sumY / CALIBRATION_SAMPLES;
  zOffset = sumZ / CALIBRATION_SAMPLES;
}

void get_magnetic_readings()
{
  motor.loopFOC();
  unsigned long currentMillis_mag = millis();

  if (currentMillis_mag - previousMillis_mag >= interval_mag) 
  {
    previousMillis_mag = currentMillis_mag;

    // read the magnetic field data
    dut.setSensitivity(TLx493D_FULL_RANGE_e); // set the sensitivity to full range
    dut.getMagneticField(&mag_x, &mag_y, &mag_z); // read the magnetic field data

    // subtract the offsets from the raw data
    mag_x -= xOffset; // The calibration offsets are subtracted to get the calibrated values.
    mag_y -= yOffset;
    mag_z -= zOffset;

    Serial.println(mag_z);

    if (DEBUG)
    {
      Serial.println("MagX , MagY , MagZ ");
      Serial.print(mag_x);
      Serial.print(",");
      Serial.print(mag_y);
      Serial.print(",");
      Serial.println(mag_z);
    }

  } 
}

void check_buttons()
{
  motor.loopFOC();
  if (digitalRead(BUTTON2) == LOW) 
  {
    control_gripper('c');

    if (DEBUG) Serial.println("Pressing Close");
  }
  else if (digitalRead(BUTTON1) == LOW) 
  {
    control_gripper('o');

    if (DEBUG) Serial.println("Pressing Open");
  }
  else 
  {
    control_gripper('s');

    if(DEBUG) Serial.println("No button pressed");
  }
}

void control_gripper(char dir)
{
  motor.loopFOC();
  if ( !flag_stop_motor && dir == 'c')
  {
    target_angle -= step_angle;

    if(DEBUG) Serial.println("Closing");
  }

  if ( dir == 'o')
  {
    target_angle += step_angle;

    flag_stop_motor = false;

    if(DEBUG) Serial.println("Opening");
  }

  if ( flag_stop_motor || dir == 's') 
  {
    target_angle += 0;
    if(DEBUG) Serial.println("Stop");
  }
}

void motor_loop() 
{
  // main FOC algorithm function
  // the faster you run this function the better
  // Arduino UNO loop  ~1kHz | Bluepill loop ~10kHz
  motor.loopFOC();

  // Motion control function
  // velocity, position or voltage (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code

  unsigned long currentMillis_mot = millis();   // get current time em ms

  if (currentMillis_mot - previousMillis_mot >= interval_mot)
  {
    motor.move(speed);
    previousMillis_mot = currentMillis_mot; // update the last time motor was moved
  
    if (DEBUG)
    {

      Serial.print("Set speed: ");
      Serial.println(speed);

    }
  }

}

void control_vel_loop()
{
  motor.loopFOC();
 
  // PID auto-tuning
  float error = target_angle - motor.shaft_angle; // error between target and actual angle
  
  if (target_angle > 6.28) target_angle -= 6.28; // keep target angle between 0 and 2*pi
  if (target_angle < 0) target_angle += 6.28;

  speed = error * speed_P;

  if (DEBUG) 
  {
    Serial.print("Target angle: ");
    Serial.println(target_angle);
    Serial.print("Actual angle: ");
    Serial.println(motor.shaft_angle);

    Serial.print("Error ");
    Serial.println(error);
  }

}

void check_pressure()
{
  motor.loopFOC();
  unsigned long currentMillis_grip = millis();

  if (currentMillis_grip - previousMillis_grip >= interval_grip) 
  {

    previousMillis_grip = currentMillis_grip;

    if (mag_z > MinGrip)
    {
      if (DEBUG) Serial.println("Object detected");

      if((mag_z-prevmagZ) < Squish) 
      {
        if (DEBUG) Serial.println("!Squishing!");
        flag_stop_motor = 1; // trava o motor a pressão atual não variar o suficiente
      }

      if (mag_z > MaxGrip)
      {
        if (DEBUG) Serial.println("!Max Grip!");
        flag_stop_motor = 1; // trava o motor quando a pressao atual exceder o limiar
      }
      prevmagZ = mag_z;
    }
  }
}

void check_serial_gui() {
  // check if data is available on serial
  while (Serial.available()) { // check if data is available
    // read the incoming byte
    char c = Serial.read();
    // check for end of line
    if (c == '\n') {
      buffer[idx] = '\0'; // null-terminate the string
      idx = 0;           // reset index for next command
      processaComando(buffer);  // Processa o comando recebido
    } else if (idx < 64 - 1) { // avoid buffer overflow
      buffer[idx++] = c; // store the character in the buffer
    }
  }
}


void processaComando(char* comando) {
  if (strcmp(comando, "A") == 0) 
  {
    control_gripper('o'); // open gripper
    if (DEBUG) Serial.println("Opening the gripper");
    strcpy(ultimo_comando, comando);
  } 
  
  else if (strcmp(comando, "F") == 0) 
  {
    control_gripper('c'); // close gripper
    if (DEBUG) Serial.println("Closing the gripper");
    strcpy(ultimo_comando, comando);
  } 
  
  else 
  {
    control_gripper('s');
    if(DEBUG) Serial.println("Comando desconhecido");
  }
}

void last_command(){
  if (strcmp(ultimo_comando, "F") == 0) {
    if (!flag_stop_motor) {
      control_gripper('c'); // continuar a fechar
    } else {
      control_gripper('s');
      strcpy(ultimo_comando, "s"); // parar
    }
  } else if (strcmp(ultimo_comando, "A") == 0) {
    if (!flag_stop_motor) {
      control_gripper('o'); // continuar a abrir
    } else {
      control_gripper('s');
      strcpy(ultimo_comando, "s"); // parar
    }
  }
}
