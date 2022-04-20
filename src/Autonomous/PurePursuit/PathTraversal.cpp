#include "robokauz/PROS.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/Autonomous/VectorMath.hpp"
#include "robokauz/Autonomous/PurePursuit/PathTraversal.hpp"
#include "robokauz/ROBOT.hpp"
#include <cfloat>

void updatePosition(TraversalCache& cache)
{
    cache.current_position = imu_odometer.getPosition();//cache.robot_properties.odom_controller->getState();
    //cache.current_position.theta = QAngle (inertial_sensor.get_rotation() * degree);
    PRINT("Position // X: " + std::to_string(cache.current_position.x.convert(foot)) + " Y: " + std::to_string(cache.current_position.y.convert(foot)));
}

void updateClosestPoint(TraversalCache& cache)
{
    QLength curr_closest_distance (DBL_MAX * meter);
    int curr_closest_index = cache.closest_index;

    for(int i = curr_closest_index; i < cache.lookahead_index + 2; i++)
    {
        QLength distance = interpointDistance(cache.current_position, cache.path.at(i));
        if(distance.convert(meter) < curr_closest_distance.convert(meter))
        {
            curr_closest_distance = distance;
            curr_closest_index = i;
        }
    }

    cache.closest_index = curr_closest_index;
    cache.closest_point = cache.path.at(curr_closest_index);

    PRINT("CP: " + std::to_string(cache.closest_index + 1) + "/" + std::to_string(cache.path.size()));
}


double findIntersect(Vector& start, Vector& end, Vector& pos, double lookahead)
{
    Vector d = end - start;
    Vector f = start - pos;

    double a = d.dot(d);
    double b = 2 * f.dot(d);
    double c = f.dot(f) - (SQ(lookahead));
    double discriminant = SQ(b) - (4.0 * (a * c));

    if(discriminant < 0)
    {
        return -1.0; //No intersection
    }
    else
    {
        discriminant = std::sqrt(discriminant);
        double t1  = (-b - discriminant) / (2 * a);
        double t2 = (-b + discriminant) / (2 * a);

        if(t2 >= 0 && t2 <= 1)
        {
            return t2;
        }

        if(t1 >= 0 && t1 <= 1)
        {
            return t1;
        }

        return -1.0; //No intersection
    }
}

void updateLookaheadPoint(TraversalCache& cache)
{
    int previous_intersection_index = 0;
    double previous_t_value = -1;

    for(int i = cache.lookahead_index; i < cache.path.size() - 1; i++)
    {
        Vector start (cache.path.at(i));
        Vector end (cache.path.at(i+1));
        Vector pos (cache.current_position);
        double t_value = findIntersect(start, end, pos, cache.path.lookahead_distance.convert(meter));
        double fractional_index = t_value + i;

        if(t_value < 0 || fractional_index <= cache.lookahead_index) { continue; }
        else if(i > cache.lookahead_index || t_value > previous_t_value)
        {   
            cache.lookahead_index = i;
            previous_t_value = t_value;

            if(previous_intersection_index > 0)
            {
                break;  
            }

            previous_intersection_index = i;

            if(previous_intersection_index > 0 && interpointDistance(cache.path.at(i), cache.path.at(previous_intersection_index)) >= cache.path.lookahead_distance * 2)
            {
                break;
            }
        }
    }

    Vector start = cache.path.at(cache.lookahead_index);
    Vector end = cache.path.at(cache.lookahead_index + 1);
    Vector d = end - start;
    Vector final_point = start + (d * previous_t_value);
    cache.lookahead_point = final_point;

    PRINT("\nLPX: " + std::to_string(cache.lookahead_point.x_component.convert(foot)));
    PRINT("LPY: " + std::to_string(cache.lookahead_point.y_component.convert(foot)));
    PRINT("LPI: " + std::to_string(cache.lookahead_index + 1));
}

void calculateCurvature(TraversalCache& cache)
{
    Vector difference = cache.lookahead_point - cache.current_position;
    Vector lookahead = cache.lookahead_point;

    QAngle constrained_angle = constrainAngle180(90_deg - cache.current_position.theta);
    double bot_angle = constrained_angle.convert(radian);
    int side = sgnum(std::sin(bot_angle) * difference.x_component.convert(meter) - std::cos(bot_angle) * difference.y_component.convert(meter));

    double a = -std::tan(bot_angle);
    double b = 1;
    double c = std::tan(bot_angle) * (cache.current_position.x.convert(meter) - cache.current_position.y.convert(meter));

    double lookahead_x = std::abs(a * lookahead.x_component.convert(meter) + lookahead.y_component.convert(meter) + c) / std::sqrt(SQ(a) + SQ(b));

    double curvature = (2 * lookahead_x) / SQ(interpointDistance(cache.current_position, lookahead).convert(meter)); 
    curvature = side * ((2 * lookahead_x) / SQ(interpointDistance(cache.current_position, cache.lookahead_point).convert(meter)));
    PRINT("Curvature: " + std::to_string(curvature));
    cache.curvature = curvature;
}

bool checkDistance(TraversalCache& cache, QLength threshold = 6_in)
{
    double distance = std::abs(interpointDistance(cache.current_position, cache.path.at(cache.path.size() - 1)).convert(inch));
    PRINT("Distance: " + std::to_string(distance));
    return distance <= threshold.convert(inch);
}

void calculateWheelSpeeds(TraversalCache &cache)
{   
    QSpeed point_velocity = cache.closest_point.velocity; 
    QSpeed target_velocity = limiter.getLimited(point_velocity, cache.robot_properties.max_acceleration);

    QSpeed left_velocity = (target_velocity.convert(mps) * (2.0 + cache.curvature * cache.robot_properties.track_width.convert(meter)) / 2.0) * mps;
    QSpeed right_velocity = (target_velocity.convert(mps) * (2.0 - cache.curvature * cache.robot_properties.track_width.convert(meter)) / 2.0) * mps;

    QAngularSpeed left_wheels = (left_velocity / (1_pi * cache.robot_properties.wheel_diam)) * 360_deg;
    QAngularSpeed right_wheels = (right_velocity / (1_pi * cache.robot_properties.wheel_diam)) * 360_deg;
    left_wheels = QAngularSpeed (std::clamp(left_wheels.convert(rpm), -200.0, 200.0) * rpm);
    right_wheels = QAngularSpeed (std::clamp(right_wheels.convert(rpm), -200.0, 200.0) * rpm);

    PRINT("\npV: " + std::to_string(point_velocity.convert(mps)));
    PRINT("tV: " + std::to_string(target_velocity.convert(mps)));
    PRINT("CVels: " + std::to_string(left_velocity.convert(mps)) + " " + std::to_string(right_velocity.convert(mps)));
    PRINT("WVels: " + std::to_string(left_wheels.convert(rpm)) + " " + std::to_string(right_wheels.convert(rpm)));

    cache.target_speeds = WheelSpeeds {left_wheels, right_wheels};
}