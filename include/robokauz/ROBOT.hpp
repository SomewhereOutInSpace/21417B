#pragma once
#include "robokauz/PRELUDE.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/CONTROLLERS.hpp"
#include "robokauz/PURE_PURSUIT.hpp"
#include "robokauz/Other/BotManager.hpp"
#include "robokauz/Other/TaskGate.hpp"
#include "robokauz/Autonomous/IMUOdometry.hpp"

enum AutonSide
{
  Left,
  Right,
  Skills,
  None
};

enum AutonStrat
{
  FullWinPoint, //Full win point auton (Not necessarily left side only)
  HalfWinPoint, //Half win point (side specific)
  SingleGoalRush,
  DoubleGoalRush 
};

extern pros::Controller master;

extern pros::Motor right_back;
extern pros::Motor right_middle;
extern pros::Motor right_front;
extern pros::Motor left_back;
extern pros::Motor left_middle;
extern pros::Motor left_front; 

extern pros::Motor arm_motor;
extern pros::Motor conveyor_motor;
extern pros::ADIDigitalOut back_piston;
extern pros::ADIDigitalOut clamp_piston;
extern pros::ADIDigitalOut top_piston;
extern pros::ADIDigitalOut lock_piston;

extern pros::IMU inertial_sensor;
extern pros::ADIEncoder left_encoder;
extern pros::ADIEncoder right_encoder;
extern pros::ADIEncoder middle_encoder;
extern pros::ADIAnalogIn potentiometer;

extern const double GEAR_RATIO;

extern LiftController arm_controller;
extern ConveyorController conveyor_controller;
extern std::shared_ptr<okapi::OdomChassisController> drive_controller;

extern EncoderGroup encoders;
extern IMUOdometer imu_odometer;

extern BotManager overwatch;
extern TaskGate driver_control_gate;

extern const RobotProperties robot_properties;
extern PathManager wayfarer;