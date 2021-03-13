#include <boxes.h>
#include <navigation.h>
#include <robot_pose.h>
#include <imagePipeline.h>

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
        ROS_INFO("Box coordinates:");

        for(int i = 0; i < boxes.coords.size(); ++i) {
            std::cout << i << "\tx: " << boxes.coords[i][0] << " y: " << boxes.coords[i][1] << " z: " 
                    << boxes.coords[i][2] << std::endl;
        }
    }

    // Initialize image objectand subscriber.
    ImagePipeline imagePipeline(n);
    // Execute strategy.
    while(ros::ok()) {
        ros::spinOnce();
        /***YOUR CODE HERE***/
        // Use: boxes.coords
        // Use: robotPose.x, robotPose.y, robotPose.phi


        // Vision stuff past here, no touchy

        // Location of test file
        char testFile[] = "/home/brobot/catkin_ws/src/mie443_contest2/testpics/crab5.png";
        imagePipeline.loadImage(testFile);
        imagePipeline.getTemplateID(boxes);

        // End of vision stuff

        ros::Duration(2).sleep(); // Two second sleep per step
    }
    return 0;
}
