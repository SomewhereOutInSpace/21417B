#pragma once

class Vector
{
    public:
        double x_component;
        double y_component;

        Vector()
        {
            x_component = 0;
            y_component = 0;
        }
        
        Vector(double x_comp, double y_comp)
        {
            x_component = x_comp;
            y_component = y_comp;
        }

        Vector(RawPoint point)
        {
            x_component = point.x_pos;
            y_component = point.y_pos;
        }

        Vector(PathPoint point)
        {
            x_component = point.x_pos;
            y_component = point.y_pos;   
        }

        Vector(RawPoint point_a, RawPoint point_b)
        {
            x_component = point_b.x_pos - point_a.x_pos;
            y_component = point_b.y_pos - point_a.y_pos; 
        }

        Vector(RobotPosition position)
        {
            x_component = position.x_pos;
            y_component = position.y_pos;
        }

        double magnitude()
        {
            double x_pow = std::pow((x_component), 2);
            double y_pow = std::pow((y_component), 2);
            return std::sqrt(x_pow + y_pow);
        }

        double direction()
        {
            double theta = std::atan2(y_component, x_component) * 180 / PI;
            return std::fmod((theta + 360), 360); 
        }

        Vector normalize()
        {
            Vector normalizedVector;
            double mag = magnitude();
            if(mag != 0)
            { 
                normalizedVector.x_component = x_component / mag;
                normalizedVector.y_component = y_component / mag;
            }
            return normalizedVector;
        }

        Vector subtract(Vector subtrahend)
        {
            Vector resultantVector;
            resultantVector.x_component = x_component - subtrahend.x_component;
            resultantVector.y_component = y_component - subtrahend.y_component;
            return resultantVector;
        }

        Vector scalarMult(double scalar)
        {
            Vector scaledVector;
            scaledVector.x_component = x_component * scalar;
            scaledVector.y_component = y_component * scalar;
            return scaledVector;
        }

        double angleBetween(Vector partner)
        {
            double angle = atan2(partner.y_component, partner.x_component) - atan2(y_component, x_component);
            angle = std::abs(angle * 180 / PI);
            return angle;
        }

        double dot(Vector multiplicand)
        {
            double angle = angleBetween(multiplicand);
            return (magnitude() * multiplicand.magnitude()) * angle;
        }
};