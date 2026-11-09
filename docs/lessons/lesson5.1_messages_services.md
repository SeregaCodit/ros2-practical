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

check **range sensor** at `I2C bus`:

<pre>
ros@rpi:/ros2-ws$ i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- 29 -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- 42 -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --     
</pre>                    

tof sensor is at 0x29 as expected.

**Bradboard connection is correct. All components are working as expected.**

