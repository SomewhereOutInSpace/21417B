#include "robokauz/PROS.hpp"
#include "robokauz/COMMON.hpp"
#include "robokauz/Autonomous/PurePursuit/PathComponents.hpp"
#include "robokauz/Autonomous/PurePursuit/PathDumping.hpp"
#include "robokauz/Autonomous/PurePursuit/PathManagement.hpp"
#include "robokauz/Autonomous/PurePursuit/PathObjects.hpp"
#include "robokauz/Autonomous/PurePursuit/PathTraversal.hpp"
#include <map>

/// --- PathTraverser --

PathTraverser::PathTraverser(Path path_to_traverse, RobotProperties robot)
{
    cache.path = path_to_traverse;
    cache.robot_properties = robot;
}

void PathTraverser::traversePath()
{
    std::uint32_t delay_timestamp = pros::millis();
    std::uint32_t dt_timestamp = pros::millis();
    while(true)
    {
        PRINT("------------ STEP! ------------");
        updatePosition(cache);
        updateClosestPoint(cache);
        updateLookaheadPoint(cache);

        cache.isOnPath = interpointDistance(cache.current_position, cache.closest_point) <= cache.path.lookahead_distance;
        cache.endWithinLookahead = interpointDistance(cache.closest_point, cache.path.end()) < 6_in &&
                                   interpointDistance(cache.current_position, cache.path.end()) < 6_in;

        calculateCurvature(cache);
        calculateWheelSpeeds(cache);

        checkDistance(cache, 1_in);
        if(cache.closest_index == cache.path.size() - 1 && checkDistance(cache, 3_in)) { break; }

        if(!cache.path.isReversed())
        {
            cache.robot_properties.odom_controller->getModel()->tank(cache.target_speeds.target_left.convert(rpm) / 200, cache.target_speeds.target_right.convert(rpm) / 200);
        }
        else
        {
            cache.robot_properties.odom_controller->getModel()->tank(-cache.target_speeds.target_left.convert(rpm) / 200, -cache.target_speeds.target_right.convert(rpm) / 200);
        }

        pros::Task::delay_until(&delay_timestamp, 20);
    }
    cache.robot_properties.odom_controller->getModel()->driveVector(0, 0);
}

// --- PathManager ---

int PathManager::getStatus()
{
    return current_status;
}

void PathManager::setStatus(int new_status)
{
    current_status = new_status;
}

void PathManager::traversePath(Path path)
{
    PathTraverser traverser(path, robot_props);
    setStatus(PurePursuitStatus::Traversing);
    traverser.traversePath();
    setStatus(PurePursuitStatus::Idle);
}

PathManager::PathManager(RobotProperties properties)
{
    robot_props = properties;
}

std::string PathManager::getPrettyStatus()
{
    switch(getStatus())
    {
        case PurePursuitStatus::Idle:
        {
            return "[#00ff00 IDLE#]";
        }
        case PurePursuitStatus::Traversing:
        {
            return "[#ffff00 TRAVERSING#]";
        }
    }
    return "[#ff0000 ERROR]";
}

PathBuilder PathManager::buildPath(std::string name, GenerationParameters g_params)
{
    return PathBuilder(name, g_params, robot_props, *this);
}

void PathManager::insertPath(Path path)
{
    stored_paths.insert(std::pair<std::string, Path>(path.getName(), path));
}

void PathManager::traverseStoredPath(std::string path_name)
{
    Path path = stored_paths[path_name];
    traversePath(path);
}

void PathManager::dumpStoredPath(std::string path_name)
{
    Path path = stored_paths[path_name];
    dumpFullPath(path);
}