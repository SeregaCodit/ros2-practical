![lesson04_sketch](../attachments/images/lesson04_sketch.png)


add `pkg-config \` то `apt-install` to command in Dockerfile and rebuild image using cache:

    docker compose up -d --build
    
make shure `GPIO17` and `GPIO27` are not used by other components or nodes:

    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 --bias=pull-up 17
    "17"=active
    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 --bias=pull-up 27
    "27"=active
