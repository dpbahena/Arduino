#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/temperature.h>

// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

rcl_publisher_t publisher;        // for linear and angular velocity
sensor_msgs__msg__Imu msg;
rclc_executor_t executor;
rcl_timer_t timer;

rcl_publisher_t temp_publisher;  //for temperature
sensor_msgs__msg__Temperature msg1;
rclc_executor_t temp_executor;
rcl_timer_t temp_timer;

rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;



#define LED_PIN 13

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    msg.linear_acceleration.x = a.acceleration.x;
    msg.linear_acceleration.y = a.acceleration.y;
    msg.linear_acceleration.z = a.acceleration.z;
    msg.angular_velocity.x = g.gyro.x;
    msg.angular_velocity.y = g.gyro.y;
    msg.angular_velocity.z = g.gyro.z;
  }
}

void temp_timer_callback(rcl_timer_t * temp_timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (temp_timer != NULL) {
    RCSOFTCHECK(rcl_publish(&temp_publisher, &msg1, NULL));
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    msg1.temperature = temp.temperature;
  }
}

void setup() {
  set_microros_transports();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  
  
  delay(2000);


  
  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_portenta_node", "", &support));

  // create publisher for IMU
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
    "imu6050"));

  // create publisher for TEMP
  RCCHECK(rclc_publisher_init_default(
    &temp_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Temperature),
    "temp6050"));
   
  
  // create timer,
  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

 // create timer for temperature,
  //const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &temp_timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    temp_timer_callback));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // create executor for temperature
  RCCHECK(rclc_executor_init(&temp_executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&temp_executor, &temp_timer));

  

  

  // Try to initialize mpu6050!
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  // Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
}

void loop() {
  delay(100);
  // imu
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
  // temp
  RCSOFTCHECK(rclc_executor_spin_some(&temp_executor, RCL_MS_TO_NS(100)));
}
