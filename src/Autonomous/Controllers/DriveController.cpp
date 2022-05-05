#include "robokauz/PRELUDE.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/Autonomous/Controllers/DriveController.hpp"

DriveController::DriveController(std::vector<int> left, std::vector<int> right)
{
    for(int port : left) { left_motors.push_back(pros::Motor(std::abs(port))); }
    for(int port : right) { right_motors.push_back(pros::Motor(std::abs(port))); }
}

void DriveController::outputLeft(int output_voltage)
{
    for(pros::Motor motor : left_motors)
    {
        motor.move_voltage(output_voltage);
    }  
}

void DriveController::outputRight(int output_voltage)
{
    for(pros::Motor motor : right_motors)
    {
        motor.move_voltage(output_voltage);
    }  
}

void DriveController::tank(int left_voltage, int right_voltage)
{
    outputLeft(left_voltage);
    outputRight(right_voltage);
}

void DriveController::splitArcade(int translation_axis_value, int rotation_axis_value)
{
    int right_output = ((translation_axis_value - rotation_axis_value) / INPUT_FACTOR) * MOTOR_MAX_VOLTAGE;
    int left_output = ((translation_axis_value + rotation_axis_value) / INPUT_FACTOR) * MOTOR_MAX_VOLTAGE;
    tank(left_output, right_output);
}

void DriveController::brake()
{
    tank(0, 0);
}