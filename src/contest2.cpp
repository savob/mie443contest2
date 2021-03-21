#include <boxes.h>
#include <navigation.h>
#include <robot_pose.h>
#include <imagePipeline.h>
#include <cmath>
#include <algorithm>
#include "fileWrite.h"
#include <time.h>
#include "tests.h"
#include "pathPlanning.h"

int main(int argc, char** argv) {
    // Setup ROS.
    ros::init(argc, argv, "contest2");
    ros::NodeHandle n;
    // Robot pose object + subscriber.
    RobotPose robotPose(0,0,0);
    ros::Subscriber amclSub = n.subscribe("/amcl_pose", 1, &RobotPose::poseCallback, &robotPose);
    // Initialize box coordinates and templates
    Boxes boxes; 
    if(!boxes.load_coords() || !boxes.load_templates()) {
        ROS_FATAL("Could not load coords or templates");
        return -1;
    }
    else {
        ROS_INFO("Box coordinates loaded successfully:");
        std::cout << "Box #, x (m), y(m), yaw (rad)\n"; // Header

        // Output data for each box nicely
        for(int i = 0; i < boxes.coords.size(); ++i) {
            char buffer[100];
            sprintf(buffer, "%2d\tx: %5.2f\ty: %5.2f\tyaw: %6.3f\n", i, 
                boxes.coords[i][0], boxes.coords[i][1], boxes.coords[i][2]);
            std::cout << buffer;
        }
    }

    // Initialize image object and subscriber.
    ImagePipeline imagePipeline(n);
    // Execute strategy.

    // Create an adjacency matrix
    int tour_points = boxes.coords.size() + 1;
    double adjMatrix[tour_points][tour_points];
    
    for(int i = 0; i < tour_points; ++i) {
        for(int j = 0; j < tour_points; ++j) {
            if(i == j){
                adjMatrix[i][j] = 0;
            }
            else if(i == 0) {
                double dx = robotPose.x - boxes.coords[j-1][0];
                double dy = robotPose.y - boxes.coords[j-1][1];
                adjMatrix[i][j] = sqrt(dx * dx + dy * dy);
            }
            else if(j == 0) {
                double dx = robotPose.x - boxes.coords[i-1][0];
                double dy = robotPose.y - boxes.coords[i-1][1];
                adjMatrix[i][j] = sqrt(dx * dx + dy * dy);
            }
            else {
                double dx = boxes.coords[i-1][0] - boxes.coords[j-1][0];
                double dy = boxes.coords[i-1][1] - boxes.coords[j-1][1];
                adjMatrix[i][j] = sqrt(dx * dx + dy * dy);
            }
        }
    }

    // Initialize vector as a set of numbers from 0 to the number of tour points
    std::vector<int> movePlan(tour_points);
    for(int i = 0; i < tour_points; ++i) {
        movePlan[i] = i;
    }

    // Prepare pointer to pass adjMatrix
    double *temp[tour_points];
    for(int i = 0; i < tour_points; ++i) temp[i] = adjMatrix[i];

    double bestScore = loopCost(temp, movePlan);
    std::vector<int> bestRoute = movePlan;
    while(std::next_permutation(movePlan.begin() + 1, movePlan.end())) {
        double s = loopCost(temp, movePlan);
        if(s < bestScore) {
            bestScore = s;
            bestRoute = movePlan;
        }
    }

    std::vector<int> boxIDs(boxes.coords.size()); // Recoding IDs of each box

    // Monitor time elapsed
    time_t startTime = time(NULL);
    int secondsElapsed = 0;
    const int timeLimit = 8 * 60; // Time limit in seconds

    while(ros::ok() && (secondsElapsed < timeLimit)) {
        ros::spinOnce();

        // =======================================================
        // Tests for features, these will only be executed once
        // Configured in "tests.h"
#ifdef FILE_WRITE_TEST
        fileWriteTest(boxes, movePlan, false);
        return 0;
#endif

#ifdef VISION_SAMPLES_TEST
        // Leave search term for vision as "" for all test cases
        visionSystemTest("pup", boxes, imagePipeline, false);
        return 0;
#endif

        // =======================================================
        // Actual loop code
        // Use: boxes.coords
        // Use: robotPose.x, robotPose.y, robotPose.phi

        // =======================================================
        // Vision code
        int ID = imagePipeline.getTemplateID(boxes, false); // Check if there is something present, do not print internals
        // NOTE: DO NOT CALL IN RAPID SUCCESSION
        // TODO: See if this is related to period between calls or quantity of calls
        if (ID == -1) {
            // Handle error... or don't
        }
        else {
            // Completed scan without event
            // 0 for blank/nothing to spot
            // >0 matches template ID spotted
            // boxIDs[current stop in path] = ID;
        }

        // End of vision stuff

        // Sleep and record elapsed time
        ros::Duration(0.1).sleep();
        secondsElapsed = time(NULL) - startTime;
    }

    // Time's up handle proper closure
    ROS_WARN("\nTIME'S UP! (%d seconds)\n RECORDING OUTPUT AND TERMINATING.\n", timeLimit);

    writeLog(boxes, movePlan, boxIDs); // Write results before closing
    return 0;
}
