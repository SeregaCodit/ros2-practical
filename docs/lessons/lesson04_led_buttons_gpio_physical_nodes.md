![lesson04_sketch](../attachments/images/lesson04_sketch.png)


## Basic `gpiod` utils

|command | destination| example|
|--------|------------|--------|
|gpiodetect|find available GPIO-controllers| gpiodetect|
|gpioinfo| view GPIO configuration and usage| gpioinfo -c gpiochip0 # for all chip<br>  gpioinfo -c gpiochip0 17 # for GPIO17 pin |
|gpoiget| read GPIO value| gpioget -c gpiochip0 -b pull-up --numeric 27 | 
|gpioset|set GPIO value| gpioset -c gpiochip0 17=1 # on<br> gpioset -c gpiochip0 17=0 # off<br>gpioset -c gpiochip0 -t 500ms 17=1 # change state eveery 500 ms (Led blinking as result)
|gpiomon|monitor signal changes (edge events)| gpiomon -c gpiochip0 -b pull-up -e both 27<br> gpiomon -c gpiochip0 -b pull-up -e both -p 30ms 27
|gpionotify|monitor GPIO configuration settings|
___



add `pkg-config \` то `apt-install` to command in Dockerfile and rebuild image using cache:

    docker compose up -d --build

make shure `GPIO17` and `GPIO27` are not used by other components or nodes:

    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 --bias=pull-up 17
    "17"=active
    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 --bias=pull-up 27
    "27"=active


start `gpio_driver` `led_node` and check manually:

<pre>
ros@rpi:/ros2-ws$ ros2 topic list
/hardware/led/set
/parameter_events
/rosout

ros@rpi:/ros2-ws$ ros2 topic list -t
/hardware/led/set [std_msgs/msg/Bool]
/parameter_events [rcl_interfaces/msg/ParameterEvent]
/rosout [rcl_interfaces/msg/Log]

ros@rpi:/ros2-ws$ ros2 interface show std_msgs/msg/Bool
# This was originally provided as an example message.
# It is deprecated as of Foxy
# It is recommended to create your own semantically meaningful message.
# However if you would like to continue using this please use the equivalent in example_msgs.

bool data

ros@rpi:/ros2-ws$ ros2 topic pub --once  /hardware/led/set std_msgs/msg/Bool "{data: true}"
publisher: beginning loop
publishing #1: std_msgs.msg.Bool(data=True)  # led lit up

ros@rpi:/ros2-ws$ ros2 topic pub --once  /hardware/led/set std_msgs/msg/Bool "{data: false}"
publisher: beginning loop
publishing #1: std_msgs.msg.Bool(data=False)  # led went out