#pragma once
#include "Enums.h"

using namespace okapi::literals;

pros::Imu inertialSensor (21);
double GEAR_RATIO = 84.0/36.0;

auto driveTrain = okapi::ChassisControllerBuilder()
  .withMotors({9, -19}, {-15, 13})
  .withGains
  (
    {0.001, 0, 0.0001}, // Distance controller gains
    {0.0035, 0.00025, 0.0001}, // Turn controller gains
    {0.6, 0.001, 0.0001}  // Angle controller gains
  )
  // Blue gearset, 4 in wheel diam, 9.5 in wheel track
  .withDimensions({okapi::AbstractMotor::gearset::blue, GEAR_RATIO}, {{4_in, 9.5_in}, okapi::imev5BlueTPR * GEAR_RATIO})
  .build();

auto pathFinder = okapi::AsyncMotionProfileControllerBuilder()
  .withLimits({2.5, 5, 10})
  .withOutput(driveTrain->getModel(), {{4_in, 9.5_in}, okapi::imev5BlueTPR * GEAR_RATIO}, {okapi::AbstractMotor::gearset::blue, GEAR_RATIO})
  .buildMotionProfileController();

//Initialize PID front lift controller
auto frontLiftRight = okapi::AsyncPosControllerBuilder()
  .withMotor(1)
  .build();

  //Initialize PID front lift controller
auto frontLiftLeft = okapi::AsyncPosControllerBuilder()
    .withMotor(5)
    .build();

  //Initialize clamp controller
  auto frontClamp = okapi::AsyncPosControllerBuilder()
    .withMotor(8)
    .build();

  //Initialize PID rear lift controller
  auto rearLift = okapi::AsyncPosControllerBuilder()
    .withMotor(6)
    .build();

  //Precomputes all Okapi paths used in skills auton.
  //Path name format: TYPE_ORIGIN_TARGET_SIDE, where:
  //TYPE is the rough purpose of the path, such as rushing/back-grabbing a goal or field traversal,
  //ORIGIN is the presumed starting point of the path,
  //TARGET is the target of the path, such as a goal or general location on the field (e.g. the start position)
  //SIDE is the field side the path is designed for. In some cases "Both" is used.
  void initSkillsPaths()
  {
    //pathFinder->
    //TBD
  }

  void initRightSimplePaths()
  {
    pathFinder->generatePath({
        {0_ft, 0_ft, 0_deg},
        {4_ft, 0_ft, 0_deg}},
        "Rush_Parking_SmallNeutral_Right"
    );
  }

  void initRightSpinPaths()
  {
    pathFinder->generatePath({
        {0_ft, 0_ft, 0_deg},
        {2_ft, 0_ft, 0_deg}},
        "Backgrab_Parking_AllianceGoal_Right"
    );
  }

  //Precomputes Okapi paths used in standard auton.
  void initPaths(int side, int strategy)
  {
      if(side == Skills) { initSkillsPaths(); return; }
      
      if(side == Right)
      {
        switch (strategy)
        {
          case SimpleRush:
          {
            initRightSimplePaths();
          }

          case SpinRush:
          {
            initRightSimplePaths();
            initRightSpinPaths();
          }

          default:
          {
            return;
          }
        }
        return;
      }
  }
