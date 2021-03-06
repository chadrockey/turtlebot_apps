/*
 * Copyright (c) 2013, Yujin Robot.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Yujin Robot nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file /include/turtlebot_panorama/panorama.h
 *
 * @brief Panorama app class definition
 *
 * @date 08/01/2013
 *
 * @author Younghoon Ju, Jihoon Lee and Marcus Liebhardt
 **/

/*****************************************************************************
 ** Ifdefs
 *****************************************************************************/

#ifndef PANORAMA_H_
#define PANORAMA_H_

#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <pano_ros/PanoCaptureAction.h>
#include <image_transport/image_transport.h>	
#include <sensor_msgs/Image.h>		
#include <std_msgs/Empty.h> 		
#include <std_msgs/String.h>
#include <nav_msgs/Odometry.h>		
#include <geometry_msgs/Twist.h>	
#include "geometry.h"
#include <turtlebot_panorama/TakePano.h>

#include <std_srvs/Empty.h>

namespace turtlebot_panorama
{

/**
 * The PanoApp utilises pano_ros for creating panorama pictures.
 */
class PanoApp
{
public:
  PanoApp();
  ~PanoApp();

  void init();
  void spin();

  /**
   * Additionally sends out logging information on a ROS topic
   * @param msg logging information
   */
  void log(std::string msg);

private:
  ros::NodeHandle nh;
  ros::NodeHandle priv_nh;
  std::map<std::string, std::string> params;
  std_msgs::Empty empty_msg;
  geometry_msgs::Twist cmd_vel, zero_cmd_vel;
  double snap_interval;
  double angle, last_angle, given_angle, ang_vel_cur;
  // panorama creation mode (continuously rotating while taking snapshots or rotate, stop, snapshot, rotate, ...)
  bool continuous;

  // public API
  // Service for starting the creation of a panorama picture
  ros::ServiceServer srv_start_pano;
  // Subscriber for starting the creation of a panorama picture
  ros::Subscriber sub_start_pano;
  // Subscriber for stopping the creation of a panorama picture
  ros::Subscriber sub_stop_pano;
  // Sends out the result of the stitched panorama picture
  image_transport::Publisher pub_stitched;

  // worker functions
  // for extra logging out via a ROS topic
  ros::Publisher pub_log;
  // for turning the robot
  ros::Publisher pub_cmd_vel;
  // for retrieving the odometry of robot
  ros::Subscriber sub_odom;

  // pano_ros API
  // client for the pano_ros action server (does the actual work)
  actionlib::SimpleActionClient<pano_ros::PanoCaptureAction>* pano_ros_client;
  // trigger snapshot taking by pano_ros
  ros::Publisher pub_action_snap;
  /**
   * for stop the pano_ros action goal; triggers the stitching of the gathered snapshots
   */
  ros::Publisher pub_action_stop;
  // recevices the stitched image from pano_ros
  image_transport::Subscriber sub_stitched;
  /**
   * turns true, when the pano_ros action goal goes active
   */
  bool is_active;
  /**
   * Tells the pano_ros feedback callback to set is_active to true (starts rotating the robot)
   * This is necessary in order to capture the first picture at the start,
   * since it takes a while to get the first pciture from the Kinect.
   */
  bool go_active;
  /**
   * Default panorama mode used for interaction via rostopic
   */
  int default_mode;
  /**
   * Default panorama angle used for interaction via rostopic
   */
  double default_pano_angle;
  /**
   * Default snap interval used for interaction via rostopic
   */
  double default_snap_interval;
  /**
   * Default rotation velocity used for interaction via rostopic
   */
  double default_rotation_velocity;

  /**
   * Starts the creation of a panorama picture via a ROS service
   * @param request specify the details for panorama creation
   * @param response the current state of the app (started, in progress, stopped)
   * @return true, if service call was successful
   */
  bool takePanoServiceCb(TakePano::Request& request, TakePano::Response& response);
//  bool takePanoServiceCb(std_srvs::Empty::Request& request, std_srvs::Empty::Response& response);

  /**
   * Simple way of taking a panorama picture (uses default values)
   * @param msg empty message
   */
  void takePanoCb(const std_msgs::EmptyConstPtr& msg);

  /**
   * Stops the panorama creation
   * @param msg empty message
   */
  void stopPanoCb(const std_msgs::EmptyConstPtr& msg);

  /**
   * Takes a snapshot
   */
  void snap();

  /**
   * Rotates the robot
   */
  void rotate();

  /**
   * Checks, if the robot has turned the specified angle interval
   */
  bool hasReachedAngle();

  /**
   * Processes the robot's odometry data
   * @param msg odometry data
   */
  void odomCb(const nav_msgs::OdometryConstPtr& msg);

  /**
   * Sends an action to goal the pano_ros action server for taking snapshots and stitching them together
   */
  void startPanoAction();

  /**
   * Stops the taking snapshots and triggers the stitching
   */
  void stopPanoAction();
  // Note: pano_ros throws an error, when it hasn't taken a snapshot yet.
  // TODO: Try to find a way to check, when stitching is possible and when the action goal needs to be cancelled.

  /**
   * Triggered when the pano_ros action goal went active
   */
  void activeCb();

  /**
   * Triggered while the pano_ros server is gathering snapshots
   * @param feedback
   */
  void feedbackCb(const pano_ros::PanoCaptureFeedbackConstPtr& feedback);

  /**
   * Triggered when the pano_ros action goal has finished
   * @param stateSNAPANDROTATE
   * @param result
   */
  void doneCb(const actionlib::SimpleClientGoalState& state, const pano_ros::PanoCaptureResultConstPtr& result);

  /**
   * Receives the stitched panorama picture
   * @param msg stiched image
   */
  void stitchedImageCb(const sensor_msgs::ImageConstPtr& msg);
};

} //namespace turtlebot_panorama

#endif /* PANORAMA_H_ */
