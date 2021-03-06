#include <controller_manager/controller_manager.h>
#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <ros/console.h>
#include "robik_hw_ifce.h"
#include "robik_util.h"
#include "robik_api.h"
#include "robot_config.h"

//arm
#include "robik/ArmControl.h"
#include "robik/VelocityControl.h"
robik::ArmControl arm_control_msg;   //message to be send to arduino, reusing memory
ros::Publisher pub_arm_control;

robik::VelocityControl velocity_control_msg;   //message to be send to arduino, reusing memory
ros::Publisher pub_velocity_control;

t_parm yaw_parm, shoulder_parm, elbow_parm, roll_parm, clamp_parm;

void init_arm_control_message() {
	arm_control_msg.header.frame_id = "base_link";
	arm_control_msg.header.stamp = ros::Time::now();
	arm_control_msg.arm_yaw = 0;
	arm_control_msg.arm_shoulder = 0;
	arm_control_msg.arm_elbow = 0;
	arm_control_msg.arm_roll = 0;
	arm_control_msg.arm_clamp = 0;
}

void statusCallback(const robik::GenericStatus& msg) {
	//arm
	RobikControllers& p_robik_controllers = RobikControllers::get_instance();

	p_robik_controllers.read_from_hw(msg);
}


void RobikControllers::init() {

    yaw_parm.fromLow = ARM_RES_MIN_YAW;
    yaw_parm.fromHigh = ARM_RES_MAX_YAW;
    yaw_parm.toLow = ARM_DEG_MIN_YAW;
    yaw_parm.toHigh = ARM_DEG_MAX_YAW;
    yaw_parm.threshold = ARM_DEG_THLD;
    yaw_parm.outlier_cnt = 0;
    yaw_parm.outlier_cnt_max = ARM_OUTL_CNT_MAX;
    yaw_parm.avg1, yaw_parm.avg2, yaw_parm.avg3 = 0;

    shoulder_parm.fromLow = ARM_RES_MIN_SHOULDER;
    shoulder_parm.fromHigh = ARM_RES_MAX_SHOULDER;
    shoulder_parm.toLow = ARM_DEG_MIN_SHOULDER;
    shoulder_parm.toHigh = ARM_DEG_MAX_SHOULDER;
    shoulder_parm.threshold = ARM_DEG_THLD;
    shoulder_parm.outlier_cnt = 0;
    shoulder_parm.outlier_cnt_max = ARM_OUTL_CNT_MAX;
    shoulder_parm.avg1, shoulder_parm.avg2, shoulder_parm.avg3 = 0;

    elbow_parm.fromLow = ARM_RES_MIN_ELBOW;
    elbow_parm.fromHigh = ARM_RES_MAX_ELBOW;
    elbow_parm.toLow = ARM_DEG_MIN_ELBOW;
    elbow_parm.toHigh = ARM_DEG_MAX_ELBOW;
    elbow_parm.threshold = ARM_DEG_THLD;
    elbow_parm.outlier_cnt = 0;
    elbow_parm.outlier_cnt_max = ARM_OUTL_CNT_MAX;
    elbow_parm.avg1, elbow_parm.avg2, elbow_parm.avg3 = 0;

    roll_parm.fromLow = ARM_RES_MIN_ROLL;
    roll_parm.fromHigh = ARM_RES_MAX_ROLL;
    roll_parm.toLow = ARM_DEG_MIN_ROLL;
    roll_parm.toHigh = ARM_DEG_MAX_ROLL;
    roll_parm.threshold = ARM_DEG_THLD;
    roll_parm.outlier_cnt = 0;
    roll_parm.outlier_cnt_max = ARM_OUTL_CNT_MAX;
    roll_parm.avg1, roll_parm.avg2, roll_parm.avg3 = 0;

    clamp_parm.fromLow = ARM_RES_MIN_CLAMP;
    clamp_parm.fromHigh = ARM_RES_MAX_CLAMP;
    clamp_parm.toLow = ARM_DEG_MIN_CLAMP;
    clamp_parm.toHigh = ARM_DEG_MAX_CLAMP;
    clamp_parm.threshold = ARM_DEG_THLD;
    clamp_parm.outlier_cnt = 0;
    clamp_parm.outlier_cnt_max = ARM_OUTL_CNT_MAX;
    clamp_parm.avg1, clamp_parm.avg2, clamp_parm.avg3 = 0;

   // connect and register the joint state interface
   hardware_interface::JointStateHandle state_handle_yaw("yaw_joint", &pos[0], &vel[0], &eff[0]);
   jnt_state_interface.registerHandle(state_handle_yaw);
   hardware_interface::JointStateHandle state_handle_shoulder("shoulder_joint", &pos[1], &vel[1], &eff[1]);
   jnt_state_interface.registerHandle(state_handle_shoulder);
   hardware_interface::JointStateHandle state_handle_elbow("elbow_joint", &pos[2], &vel[2], &eff[2]);
   jnt_state_interface.registerHandle(state_handle_elbow);
   hardware_interface::JointStateHandle state_handle_roll("roll_joint", &pos[3], &vel[3], &eff[3]);
   jnt_state_interface.registerHandle(state_handle_roll);
   hardware_interface::JointStateHandle state_handle_clamp("clamp_joint", &pos[4], &vel[4], &eff[4]);
   jnt_state_interface.registerHandle(state_handle_clamp);

   hardware_interface::JointStateHandle state_handle_left_wheel("wheel_left_joint",&pos[5],&vel[5],&eff[5]);
   jnt_state_interface.registerHandle(state_handle_left_wheel);
   hardware_interface::JointStateHandle state_handle_right_wheel("wheel_right_joint",&pos[6],&vel[6],&eff[6]);
   jnt_state_interface.registerHandle(state_handle_right_wheel);
   registerInterface(&jnt_state_interface);

   // connect and register the joint position interface
   hardware_interface::JointHandle pos_handle_yaw(jnt_state_interface.getHandle("yaw_joint"), &cmd[0]);
   jnt_pos_interface.registerHandle(pos_handle_yaw);
   hardware_interface::JointHandle pos_handle_shoulder(jnt_state_interface.getHandle("shoulder_joint"), &cmd[1]);
   jnt_pos_interface.registerHandle(pos_handle_shoulder);
   hardware_interface::JointHandle pos_handle_elbow(jnt_state_interface.getHandle("elbow_joint"), &cmd[2]);
   jnt_pos_interface.registerHandle(pos_handle_elbow);
   hardware_interface::JointHandle pos_handle_roll(jnt_state_interface.getHandle("roll_joint"), &cmd[3]);
   jnt_pos_interface.registerHandle(pos_handle_roll);
   hardware_interface::JointHandle pos_handle_clamp(jnt_state_interface.getHandle("clamp_joint"), &cmd[4]);
   jnt_pos_interface.registerHandle(pos_handle_clamp);
   registerInterface(&jnt_pos_interface);

	hardware_interface::JointHandle vel_handle_left_wheel(jnt_state_interface.getHandle("wheel_left_joint"), &cmd[5]);
	jnt_vel_interface.registerHandle(vel_handle_left_wheel);
	hardware_interface::JointHandle vel_handle_right_wheel(jnt_state_interface.getHandle("wheel_right_joint"), &cmd[6]);
	jnt_vel_interface.registerHandle(vel_handle_right_wheel);
	registerInterface(&jnt_vel_interface);

	//IMU
	imu_data.name = "IMU";
	imu_data.frame_id = "base_link";
	imu_data.orientation = imu_orientation;
	imu_data.angular_velocity = imu_angular_velocity;
	imu_data.linear_acceleration = imu_linear_acceleration;
	hardware_interface::ImuSensorHandle imu_sensor_handle(imu_data);
	imu_interface.registerHandle(imu_sensor_handle);
	registerInterface(&imu_interface);
}

void RobikControllers::read_from_hw (const robik::GenericStatus& msg) {

	//Arm joint state
	if (msg.arm_enabled == 1) {
		pos[0] = map_avgcheck_inf(msg.arm_yaw, &yaw_parm) * DEG_TO_RAD;
		pos[1] = map_avgcheck_inf(msg.arm_shoulder, &shoulder_parm) * DEG_TO_RAD;
		pos[2] = map_avgcheck_inf(msg.arm_elbow, &elbow_parm) * DEG_TO_RAD;
		pos[3] = map_avgcheck_inf(msg.arm_roll, &roll_parm) * DEG_TO_RAD;
		pos[4] = map_avgcheck_inf(msg.arm_clamp, &clamp_parm) * DEG_TO_RAD;
	}
	if (pos[0] == 0)
		ROS_INFO("sub arm_control %f %f %f %f %f", pos[0],pos[1],pos[2],pos[3],pos[4]);

	//IMU
	imu_orientation[0] = msg.imu_compass_v3_x;
	imu_orientation[1] = msg.imu_compass_v3_y;
	imu_orientation[2] = msg.imu_compass_v3_z;
	imu_orientation[3] = 1;  //TODO Is this correct to assume 1?
	imu_angular_velocity[0] = msg.imu_angular_velocity_v3_x;
	imu_angular_velocity[1] = msg.imu_angular_velocity_v3_y;
	imu_angular_velocity[2] = msg.imu_angular_velocity_v3_z;
	imu_linear_acceleration[0] = msg.imu_linear_acceleration_v3_x;
	imu_linear_acceleration[1] = msg.imu_linear_acceleration_v3_y;
	imu_linear_acceleration[2] = msg.imu_linear_acceleration_v3_z;

	//Odom
	#define MULTIPLIER 11.5
	pos[5] += msg.odom_ticksLeft  * (TICK_LENGTH_MM / 1000) * MULTIPLIER;  //traveled distance [m]
	pos[6] += msg.odom_ticksRight * (TICK_LENGTH_MM / 1000) * MULTIPLIER;
	vel[5] = 0;  //not used by diff_drive odometry.cpp
	vel[6] = 0;
	eff[5] = 0;
	eff[6] = 0;

}

void RobikControllers::write_to_hw(){
	//arm
	arm_control_msg.header.stamp = ros::Time::now();
	arm_control_msg.arm_yaw = (unsigned int) map_unchecked(cmd[0] * RAD_TO_DEG, ARM_DEG_MIN_YAW, ARM_DEG_MAX_YAW, ARM_MIN_YAW, ARM_MAX_YAW);
	arm_control_msg.arm_shoulder = (unsigned int) map_unchecked(cmd[1] * RAD_TO_DEG, ARM_DEG_MIN_SHOULDER, ARM_DEG_MAX_SHOULDER, ARM_MIN_SHOULDER, ARM_MAX_SHOULDER);
	arm_control_msg.arm_elbow = (unsigned int) map_unchecked(cmd[2] * RAD_TO_DEG, ARM_DEG_MIN_ELBOW, ARM_DEG_MAX_ELBOW, ARM_MIN_ELBOW, ARM_MAX_ELBOW);
	arm_control_msg.arm_roll = (unsigned int) map_unchecked(cmd[3] * RAD_TO_DEG, ARM_DEG_MIN_ROLL, ARM_DEG_MAX_ROLL, ARM_MIN_ROLL, ARM_MAX_ROLL);
	arm_control_msg.arm_clamp = (unsigned int) map_unchecked(cmd[4] * RAD_TO_DEG, ARM_DEG_MIN_CLAMP, ARM_DEG_MAX_CLAMP, ARM_MIN_CLAMP, ARM_MAX_CLAMP);
	arm_control_msg.time_to_complete = 5; //50ms ~ 20Hz in milliseconds, this is frequency of /robik_arm_control

//	ROS_INFO("pub arm_control %f %f %f %f %f", cmd[0],cmd[1],cmd[2],cmd[3],cmd[4]);
//	ROS_INFO("pub arm_control %u %u %u %u", arm_control_msg.arm_yaw, arm_control_msg.arm_shoulder, arm_control_msg.arm_elbow, arm_control_msg.arm_roll);
	pub_arm_control.publish(arm_control_msg);

	//Velocity
	velocity_control_msg.header.stamp = ros::Time::now();
	velocity_control_msg.motor_left = cmd[5] / 10;
	velocity_control_msg.motor_right = cmd[6] / 10;
	pub_velocity_control.publish(velocity_control_msg);

}

ros::Time RobikControllers::get_time(){
	return ros::Time::now();
}

ros::Duration RobikControllers::get_period(){
	ros::Time current_time = ros::Time::now();
	ros::Duration period = current_time - last_time;
	last_time = current_time;
	return period;
}

int main(int argc, char **argv) {

	ros::init(argc, argv, "robik_hw_ifce");
	ros::NodeHandle n;
	RobikControllers& robik_controllers = RobikControllers::get_instance();
	robik_controllers.init();
	init_arm_control_message();
	pub_arm_control = n.advertise<robik::ArmControl>("robik_arm_control", 20);
	pub_velocity_control = n.advertise<robik::VelocityControl>("robik_velocity_control", 20);

	ros::Subscriber sub_status = n.subscribe("robik_status", 100, statusCallback);


	ros::CallbackQueue my_callback_queue;
	n.setCallbackQueue(&my_callback_queue);
	controller_manager::ControllerManager cm(&robik_controllers, n);
	ros::AsyncSpinner spinner(0, &my_callback_queue);
	spinner.start();

	bool initialized = false;
	unsigned int cnt = 0;
	ros::Time start = robik_controllers.get_time();
	while (ros::ok()) {
		if (!initialized) {
			if (cnt < 100)
				cnt++;
			else {
				initialized = true;
			}

		}
		cm.update(robik_controllers.get_time(), robik_controllers.get_period());
		if (initialized) robik_controllers.write_to_hw();
		nanosleep((const struct timespec[]){{0, 100000000L}}, NULL); //50ms ~ 20Hz
		ros::spinOnce(); //in case of issue with not responding on commands, try to remove this but statusCallBack will not be called anymore
	}
	return 0;
}
