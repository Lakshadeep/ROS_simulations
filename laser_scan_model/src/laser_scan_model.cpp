#include "ros/ros.h"
#include <sstream>
#include "geometry_msgs/Pose2D.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/OccupancyGrid.h"

#define degtorad 3.1457/180

float map[758][984];

void mapcallback (const nav_msgs::OccupancyGrid::ConstPtr& map_in)
{
  // int size = 758*984;
  int i,j;
  
  for(i = 0; i < 441; i++ ){
    for(j = 0; j < 332; j++ ){
      map[i][j] = map_in->data[332 * i + j];
      // ROS_INFO("Map in callback %f", map[i][j]);
    }
  }
}
int main(int argc, char** argv){
  ros::init(argc, argv, "laser_scan_publisher");

  ros::NodeHandle n;
  ros::Publisher scan_pub = n.advertise<sensor_msgs::LaserScan>("scan", 50);
  ros::Subscriber sub1 = n.subscribe("map", 10, mapcallback);

  unsigned int num_readings = 72;
  double laser_frequency = 40;
  double ranges[num_readings];
  double intensities[num_readings];

  int count = 0;
  ros::Rate r(1);
  while(n.ok()){
    //generate some fake data for our laser scan
    for(unsigned int i = 0; i < num_readings; ++i){
      ranges[i] = i * 0.1;
      intensities[i] = 100 + count;
    }
    ros::Time scan_time = ros::Time::now();

    //populate the LaserScan message
    sensor_msgs::LaserScan scan;
    scan.header.stamp = scan_time;
    scan.header.frame_id = "odom";
    scan.angle_min = -0.785;
    scan.angle_max = 5.495;
    scan.angle_increment = 6.28 / num_readings;
    scan.time_increment = (1 / laser_frequency) / (num_readings);
    scan.range_min = 0.0;
    scan.range_max = 5.0;

    scan.ranges.resize(num_readings);
    scan.intensities.resize(num_readings);
    
    float th;
    float a = 100.0;
    for(unsigned int i = 0; i < num_readings; i++){
      int dist;
      float x = 0,y = 431;
      th = (i * 6.28/ num_readings);

      ROS_WARN("Theta %f", th * 180/3.14);

      
      while( 101 > map[abs(x)][abs(y)] || map[abs(x)][abs(y)] < 99){
        // ROS_INFO("Map %d", map[abs(x)][abs(y)]);
        x = x + (0.01 * cos(th) - 0.01 * sin(th));
        y = y + (0.01 * sin(th) + 0.01 * cos(th));
        // ROS_INFO("Co-ordinates %d %d", abs(x),abs(y));

        if(x >= 332 || x <= 0) break;
        if(y >= 431 || y <= 0) break;
      }

      if(map[abs(x)][abs(y)] == -1){
        scan.ranges[i] = 10;
      }else{ 
        scan.ranges[i] = sqrt(pow(x,2) + pow(y - 431,2))/100;
        ROS_INFO("Distance %f", scan.ranges[i]);
        ROS_INFO("X = %f, Y = %f ",x,y);
        ROS_INFO("Map %f", map[abs(x)][abs(y)]);
        ROS_INFO(" ");
      }
      // else 
        // scan.ranges[i] = 100/100;
      // if(num_readings == 36) 
      //   scan.ranges[i] = 1.5;
      // else 
      //   scan.ranges[i] = 100/100;
    
      // ROS_INFO("map %f", map[abs(x)][abs(y)]);
      scan.intensities[i] = 0;
    }

    scan_pub.publish(scan);
    ++count;
    ros::spinOnce();
    r.sleep();
  }
}