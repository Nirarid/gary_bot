/*
* 29-nov-2021 05:48
* Working modules: TeleOP
*                  Follower
*                  Slow hector mapping
*                  Avoidance
* 
* Need fix:        Sensor Process
* 
* To Do:           ---
*/

#include "Arduino.h"
#include <ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Bool.h>
#include <geometry_msgs/Twist.h>

//Joystick Pins
int x_key = A0;
int y_key = A1;
int x_pos;
int y_pos;
const int SWPin = 43;

//Motor Pins
int EN_A = 11; //Enable pin for first motor
int IN1 = 2; //control pin for first motor
int IN2 = 3; //control pin for first motor
int IN3 = 6; //control pin for second motor
int IN4 = 7; //control pin for second motor
int EN_B = 10; //Enable pin for second motor
int seeder = 13; //control pin for seeder motor
int Trigger = 45; //Sensor trigger pin
int Echo = 44; //Sensor echo pin
int MA = 9; //Control pin for Manual/Auto controller
int Stop = 12; //Control pin for emergency stop

//Initializing variables to store data
//Speed variables
int manual_speed1=0;
int manual_speed2=0;
int auto_speed1=0;
int auto_speed2=0;
int aux_line;
int aux_angle;
//Mode switch variables
int JoySW = 1;
bool edgeFind = false;
std_msgs::Int32 mode;
//Sensor variables
long duration;
float distance;
int readings;
std_msgs::Bool empty;

//Create the ROS subscribers and handles
ros::NodeHandle nh;


void ros_controller_cb(const geometry_msgs::Twist & msg){
  if(mode.data == 0){  //Execute teleoperated mode
    if(msg.angular.z > 0.0){
      //gira izquierda
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      aux_angle = (int)100*msg.angular.z;
      auto_speed1 = (aux_angle>255) ? 255 : aux_angle; //Read ROS speed or force 255 as top speed
    
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = (aux_angle>255) ? 255 : aux_angle;
    }
    else if(msg.angular.z < 0.0){
      //gira derecha
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_angle = (int)100*msg.angular.z;
      aux_angle = abs(aux_angle);
      auto_speed1 = (aux_angle>255) ? 255 : aux_angle; //Read ROS speed or force 255 as top speed
    
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      auto_speed2 = (aux_angle>255) ? 255 : aux_angle;
    }
    else if((msg.linear.x == 0.0) && (msg.angular.z == 0.0)){
      //stop
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }
    else if(msg.linear.x > 0.0){
      //adelante
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_line = (int)100*msg.linear.x;
      auto_speed1 = (aux_line>255) ? 255 : aux_line; //Read ROS speed or force 255 as top speed
        
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = (aux_line>255) ? 255 : aux_line;
    }
    else if(msg.linear.x < 0.0){
      //atras
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      aux_line = (int)100*msg.linear.x;
      aux_line = abs(aux_line);
      auto_speed1 = (aux_line>255) ? 255 : aux_line; //Read ROS speed or force 255 as top speed
        
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      auto_speed2 = (aux_line>255) ? 255 : aux_line;
    }
  }
  else if(mode.data == 1){      //Execute follower mode
    if(msg.angular.z > 0.2){
      //gira izquierda
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      aux_angle = (int)100*msg.angular.z;
      auto_speed1 = map(aux_angle, 20, 145, 150, 255); //Mapping the values to 150-255 to move the motor
    
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = map(aux_angle, 20, 145, 150, 255);
    }
    else if(msg.angular.z < -0.2){
      //gira derecha
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_angle = (int)100*msg.angular.z;
      aux_angle = abs(aux_angle);
      auto_speed1 = map(aux_angle, 20, 145, 150, 255); //Mapping the values to 150-255 to move the motor
    
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      auto_speed2 = map(aux_angle, 20, 145, 150, 255);
    }
    else if((msg.linear.x == 0.0) && (msg.angular.z > -0.2) && (msg.angular.z < 0.2)){
      //stop
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }
    else if(msg.linear.x > 0.0){
      //adelante
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_line = (int)100*msg.linear.x;
      auto_speed1 = map(aux_line, 0, 100, 50, 200); //Mapping the values to 50-200 to move the motor
        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
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = map(aux_line, 0, 100, 50, 200);
    }
  }
  else {    //Execute avoidance mode
    if(msg.angular.z > 0.15){
      //gira izquierda
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      aux_angle = (int)100*msg.angular.z;
      aux_angle = (aux_angle>150) ? 150 : aux_angle; //Read ROS speed or force 150 as top speed
      auto_speed1 = map(aux_angle, 0, 150, 130, 255); //Mapping the values to 130-255 to move the motor
      
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = map(aux_angle, 0, 150, 130, 255);
    }
    else if(msg.angular.z < -0.15){
      //gira derecha
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_angle = (int)100*msg.angular.z;
      aux_angle = abs(aux_angle);
      aux_angle = (aux_angle>150) ? 150 : aux_angle; //Read ROS speed or force 150 as top speed
      auto_speed1 = map(aux_angle, 0, 150, 130, 255); //Mapping the values to 130-255 to move the motor
      
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      auto_speed2 = map(aux_angle, 0, 150, 130, 255);
    }
    else if((msg.linear.x == 0.0) && (msg.angular.z > -0.05) && (msg.angular.z < 0.05)){
      //stop
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }
    else if(msg.linear.x > 0.0){
      //adelante
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      aux_line = (int)100*msg.linear.x;
      aux_line = (aux_line>150) ? 150 : aux_line; //Read ROS speed or force 150 as top speed
      auto_speed1 = map(aux_line, 0, 150, 50, 255); //Mapping the values to 50-255 to move the motor
          
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      auto_speed2 = map(aux_line, 0, 150, 50, 255);
    }
  }
}

void manual_controller_cb(){
  if(y_pos<450){
    //adelante
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    manual_speed1 = map(y_pos, 450, 0, 0, 255); //Mapping the values to 0-255 to move the motor
    
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    manual_speed2 = map(y_pos, 450, 0, 0, 255);
  }
  else if(y_pos>570){
    //atrás
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    manual_speed1 = map(y_pos, 570, 1023, 0, 255); //Mapping the values to 0-255 to move the motor
  
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    manual_speed2 = map(y_pos, 570, 1023, 0, 255);
  }
  else if(x_pos<450){
    //gira izquierda
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    manual_speed1 = map(x_pos, 450, 0, 0, 255); //Mapping the values to 0-255 to move the motor
  
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    manual_speed2 = map(x_pos, 450, 0, 0, 255);
  }
  else if(x_pos>570){
    //gira derecha
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    manual_speed1 = map(x_pos, 570, 1023, 0, 255); //Mapping the values to 0-255 to move the motor
  
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    manual_speed2 = map(x_pos, 570, 1023, 0, 255);
  }
  else if((y_pos>450)&&(y_pos<570)&&(x_pos>450)&&(x_pos<570)){
    //stop
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
  Serial.print("X: ");
  Serial.print(x_pos);
  Serial.print(", Y: ");
  Serial.println(y_pos);
}


ros::Subscriber<geometry_msgs::Twist> cmd_vel_sub("arduino/cmd_vel", ros_controller_cb);
ros::Publisher mode_notify("cmd_mode", &mode);
ros::Publisher empty_notify("meow_sensor", &empty);

void setup ( ) {
  Serial.begin (57600); //Starting the serial communication at 9600 baud rate
  //init ROS communication
  nh.initNode();
  //Subscibed ROS topics
  nh.subscribe(cmd_vel_sub);
  //Advertise publisher
  nh.advertise(mode_notify);
  nh.advertise(empty_notify);
  
  //Initializing the motor pins as output
  pinMode(EN_A, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN_B, OUTPUT);

  //Initializng the joystick pins as input
  pinMode(x_key, INPUT);
  pinMode(y_key, INPUT);
  pinMode(SWPin,INPUT_PULLUP);

  //Initializing PORT for Auto/Manual mode switch
  pinMode(MA, INPUT);

  //Initializing PORTs for seeding mechs
  pinMode(Trigger,OUTPUT);
  pinMode(Echo,INPUT);
  pinMode(seeder,OUTPUT);
  readings = 0;

  //Variables for toggling modes
  JoySW = 1;
  mode.data = 0;
  edgeFind = false;
  empty.data = false;
  mode_notify.publish(&mode);
}

void loop () {
  nh.spinOnce();
  if(digitalRead(MA) == HIGH){
    //Manual Processing
    x_pos = analogRead(x_key) ; //Reading the horizontal movement value
    y_pos = analogRead(y_key) ; //Reading the vertical movement value
    manual_controller_cb();     //Execute manual mode
    analogWrite(EN_A, manual_speed1);
    analogWrite(EN_B, manual_speed2);
  }
  else {
    analogWrite(EN_A, auto_speed1);
    analogWrite(EN_B, auto_speed2);
    //Read Joystick switch for toggle modes
    // 0 - Teleoperated
    // 1 - Follower
    // 2 - Avoidance
    JoySW = digitalRead(SWPin);
    if(digitalRead(MA) == LOW && JoySW == 0){
      delay(50);
      if(!edgeFind){
        if(mode.data == 2) mode.data = 0;
        else mode.data += 1;
        edgeFind = true;
        mode_notify.publish(&mode);
      }
      else {}
    }
    else {
      edgeFind = false;
    }
  }
  //Sensor Processing
  digitalWrite(Trigger,LOW);
  delayMicroseconds(2);
  digitalWrite(Trigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigger,LOW);
  duration = pulseIn(Echo,HIGH);
  distance = 0.034*duration/2;
  //Protect functions from misreadings
  //Serial.println(distance);
  if(distance < 11){
    readings = (readings<5) ? readings+1 : readings;
  }
  else if (distance >= 11){
    readings = (readings>0) ? readings-1 : readings;
  }
  else{}
  if(readings == 5){
    if(empty.data){
      empty_notify.publish(&empty);
      digitalWrite(seeder, LOW);
      empty.data = false;
    }
  }
  else if(readings == 0){
    digitalWrite(seeder, HIGH);
    empty.data = true;
  }
  else {}
}
