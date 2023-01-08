#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>

//rcl_subscription_t subscriber;
rcl_publisher_t color_pub_;
std_msgs__msg__String color_msg;
std_msgs__msg__Int32 msg;
//rclc_executor_t executor;  // execute subscriber
rclc_executor_t executor1;  // execute publisher
rclc_support_t support;
rcl_allocator_t allocator;

rcl_node_t node;
rcl_timer_t timer; // needed for publisher

#define LED_PIN 13
// #define LED_PIN1 LEDB
// #define LED_PIN2 LEDG

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

// void subscription_callback(const void * msgin)
// {  
//   const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
//   //digitalWrite(LED_PIN, (msg->data == 0) ? LOW : HIGH);
//   if (msg->data == 0){
//     digitalWrite(LED_PIN, LOW);
//     digitalWrite(LED_PIN1, HIGH);
//     digitalWrite(LED_PIN2, HIGH);
//     color_msg.data.data = "Red";
//   }else if (msg->data > 0 && msg->data < 100){
//     digitalWrite(LED_PIN1,LOW);
//     digitalWrite(LED_PIN,HIGH);
//     digitalWrite(LED_PIN2,HIGH);
//     color_msg.data.data = "Blue"; 
//     //strcpy(color_msg.data.data, "BlueTest");
//   }else{
//     digitalWrite(LED_PIN2,LOW);
//     digitalWrite(LED_PIN1,HIGH);
//     digitalWrite(LED_PIN,HIGH);
//     color_msg.data.data = "Green";
//   }    
    
// }
void timer_callback(rcl_timer_t * timer, int64_t last_call_time){
  RCLC_UNUSED(last_call_time);
  if(timer !=  NULL){
    
    RCSOFTCHECK(rcl_publish(&color_pub_, &color_msg, NULL));

  }
}
void setup() {
  set_microros_transports();
  
  pinMode(LED_PIN, OUTPUT);
  // pinMode(LED_PIN1, OUTPUT);
  // pinMode(LED_PIN2, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  
  // digitalWrite(LED_PIN1, HIGH);
  // digitalWrite(LED_PIN2, HIGH);
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

  // create subscriber
  // RCCHECK(rclc_subscription_init_default(
  //   &subscriber,
  //   &node,
  //   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
  //   "display_weights"));

  // create publisher 
  RCCHECK(rclc_publisher_init_default(
    &color_pub_,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "color_topic"
  ));
  // create timer,
  const unsigned int timer_timeout = 100;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback
  ));

  // create executor
  //RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_init(&executor1, &support.context, 1, &allocator));
  //RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor1, &timer));
  color_msg.data.data = "";
}

void loop() {
  delay(100);
  //RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
  RCCHECK(rclc_executor_spin_some(&executor1, RCL_MS_TO_NS(100)));

}
