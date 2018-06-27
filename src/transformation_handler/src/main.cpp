#include "ros/ros.h"
#include "geometry_msgs/Point.h"
#include <math.h>
double X = -1;
double Y = -1;
double Z = -1;
double rotateY;
double rotateZ;
double vx = 0;
double vy = 0;
double vz = 0;
double cc[4] = {X,Y,Z,1};
double tm[4][4] = {{1,0,0,vx},{0,1,0,vy},{0,0,1,vz},{0,0,0,1}};
void transformationHandlerCallback(const geometry_msgs::Point::ConstPtr& msg)
{
	X = msg->x;
	Y = msg->y;
	Z = msg->z;
	ROS_INFO("received location: %f %f %f",X,Y,Z);
}

int main(int argc,char** argv)
{
	ros::init(argc,argv,"transformationHandler");
	ros::NodeHandle n;
	ros::Publisher angle_pub = n.advertise<geometry_msgs::Point>("gimbal_angles",1);
	geometry_msgs::Point angles;
	angles.x = 0;
	ros::Subscriber sub = n.subscribe("rune_locations", 1, transformationHandlerCallback);
	double gx = 0;
 	double gy = 0;
	double gz = 0;
	for(int i = 0; i<4;i++)
	{	
	   gx += tm[0][i]; 
           gy += tm[1][i];
	   gz += tm[2][i];
	}
	angles.z = atan(gy/gx);
	angles.y = asin(gz/sqrt(gx*gx+gy*gy+gz*gz));
	angle_pub.publish(angles);
	ROS_INFO("publish angles: %f %f",angles.z,angles.y);
 	ros::spin();
	return 0;
}
