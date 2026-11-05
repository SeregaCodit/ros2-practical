![lesson04_sketch](../attachments/images/lesson04_sketch.png)


## Basic `gpiod` utils

|command | destination| example|
|--------|------------|--------|
|gpiodetect|find available GPIO-controllers| gpiodetect|
|gpioinfo| view GPIO configuration and usage| gpioinfo -c gpiochip0 # for all chip<br>  gpioinfo -c gpiochip0 17 # for GPIO17 pin |
|gpoiget| read GPIO value| gpioset -c gpiochip0 17=1 # on<br> gpioset -c gpiochip0 17=0 # off<br>gpioset -c gpiochip0 -t 500ms 17=1 # change state eveery 500 ms (Led blinking as result)|
|gpioset|set GPIO value| gpioget -c gpiochip0 -b pull-up --numeric 27 | 
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
