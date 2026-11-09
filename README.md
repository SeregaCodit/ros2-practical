# ROS 2 Practical

Practical robotics course project.

## Technology stack

- Ubuntu 26.04
- ROS 2 Lyrical
- Gazebo Jetty
- Docker Compose
- Python and C++

## Start environment

    docker compose up -d --build
    docker compose exec ros2 bash

## ROS check
    
    ros2 run demo_nodes_cpp talker     # in first terminal
    ros2 run demo_nodes_py listener    # in second terminal

## Workspace - [ros2-ws/src](ros2-ws)

## Hardware inventory
| Component | Usage |
|-----------|-------|
|Raspberry Pi 4B, 8 GB | main computer for ROS 2|
|Waveshare UPS HAT (B) |  Raspberry Pi power supply, I2C- monitoring|
|Arduino Nano |	simple controller for motors, encoders and low-level sensors|
| 2 x TT Motor 1:90, 3–6 V | differential drive|
| 2 x Optical encoders | wheel speed measurement, closed-loop (PID), odometry| 
|motor driver TB6612FNG| motor driver|
| ToF VL53L0X, Ultrasonic HC-SR04 |	Obstacle detection and distance experiments|
|Raspberry Pi Camera Module 3 |	Computer Vision and ROS image pipeline|
|Matek H7A3-WING |	final low-level controller|
|ELRS-Reciver |	RC, mode switching, failsafe|
|Servos, LED, buttons, breadboard |  Specific mechanisms and laboratory exercises on drive control|
|6S LiPo battery, DC-DC stepdown | Power system operation with appropriate voltage transformation|
___


### notes from lessons 
* [lesson02 - ros_graph packages pubsub foxglove_bridge](docs/lessons/lesson02_ros_graph_packages_pubsub.md)
* [lesson03 - RPI pinout and voltage](docs/lessons/lesson03_rpi_pins.md)
* [lesson04 - LED_buttons_GPIO_physical_nodes](docs/lessons/lesson04_led_buttons_gpio_physical_nodes.md)
* [lesson5.1_messages_services](docs/lessons/lesson5.1_messages_services.md)

___
## [troubleshooting](docs/troublesootings/troubleshooting.md)