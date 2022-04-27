#include "robokauz/PRELUDE.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/ROBOT.hpp"
#include "robokauz/CONTROLLERS.hpp"
#include "robokauz/Autonomous/VectorMath.hpp"
#include "robokauz/Autonomous/IMUOdometry.hpp"

//TODO write PID loops using continuous odom feedback. 

//Need at minimum:
//Turn in place (relative and absolute)
//Turn to face point
//Drive to point (which subroutines turning to face a point)

//Would be cool-slash-nice to have:
//Drive to point in arc

QAngle getRobotHeading()
{
    return (inertial_sensor.get_rotation()) * degree;
}

void turnRelative(QAngle target_angle)
{
	double error = target_angle.convert(degree) - getRobotHeading().convert(degree);
    double threshold = 1.0;
	double integral;
	double derivative;
	double prevError;
	double kp = 1.2;
	double ki = 0.0025;
	double kd = 7.5;

    while(fabs(error) > threshold)
    {  
        error = target_angle.convert(degree) - getRobotHeading().convert(degree);
        integral = integral + error;

		if(error == 0 || fabs(error) >= target_angle.convert(degree))
        {
			integral = 0;
		}

		derivative = error - prevError;
		prevError = error;
		double p = error * kp;
		double i = integral * ki;
		double d = derivative * kd;

		double vel = p + i + d;

		right_back.move_velocity(-vel);
        right_middle.move_velocity(-vel);
        right_front.move_velocity(-vel);

        left_back.move_velocity(vel);
        left_middle.move_velocity(vel);
        left_front.move_velocity(vel);

		pros::delay(15);
	}
    drive_train->getModel()->driveVector(0, 0);
}

void turnAbsolute(QAngle total_angle)
{
    QAngle absolute_angle = QAngle ((getRobotHeading().convert(degree) + total_angle.convert(degree)) * degree);
    absolute_angle = constrainAngle360(absolute_angle);
    turnRelative(absolute_angle);
}

void turnToPoint(Point point)
{
    auto odom_state = drive_train->getState();
    double x_component = point.x.convert(meter) - odom_state.x.convert(meter);
    double y_component = point.y.convert(meter) - odom_state.y.convert(meter);
    double output = std::atan2(y_component, x_component) * 180 / pi;
    output = std::fmod((output + 360), 360);
    QAngle target_angle = QAngle(output * degree);
    target_angle = constrainAngle360(target_angle);
    turnRelative(target_angle);
}