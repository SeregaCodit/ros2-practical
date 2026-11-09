## Breadboard 
![lesson5.1_breadboard](../attachments/images/lesson5.1_breadboard.png)

### check connections and pins
we know that the gpiochip name is `gpiochip0` from previous lesson

 check button:

    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 -b pull-up 17
    "17"=active
    ros@rpi:/ros2-ws$ gpioget -c gpiochip0 -b pull-up 17
    "17"=inactive

check **red LED** at `GPIO27`:

    ros@rpi:/ros2-ws$ gpioset -c gpiochip0 27=1  # lit up
    ^C
    ros@rpi:/ros2-ws$ gpioset -c gpiochip0 27=0  # went out
    ^C

check **green LED** at `GPIO22`:

    ros@rpi:/ros2-ws$ gpioset -c gpiochip0 22=1  # lit up
    ^C
    ros@rpi:/ros2-ws$ gpioset -c gpiochip0 22=0  # went out
    ^C
