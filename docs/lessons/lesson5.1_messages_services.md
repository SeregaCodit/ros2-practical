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

# Interfaces
after building  [safety_interfaces](../../ros2-ws/src/lessons/lesson_5.1/safety_interfaces) package check them:
<pre>
ros@rpi:/ros2-ws$ ros2 interface list | grep safety_interfaces
    safety_interfaces/msg/ArmDecision
    safety_interfaces/msg/ArmState
    safety_interfaces/msg/ButtonState
    safety_interfaces/msg/LedCommand
    safety_interfaces/srv/SetArm

ros@rpi:/ros2-ws$ ros2 interface show safety_interfaces/msg/ArmState
uint8 STANDBY=0
uint8 ARMED=1

builtin_interfaces/Time stamp
	int32 sec
	uint32 nanosec
uint8 state
uint32 revision
# revision incrases by 1 at each state chage


ros@rpi:/ros2-ws$ ros2 interface show safety_interfaces/srv/SetArm 
uint8 ARM=1
uint8 DISARM=2

uint8 command

---

safety_interfaces/ArmDecision decision
	uint8 OK=0
	uint8 INVALID_COMMAND=1
	uint8 UPS_UNAVAILABLE=2
	uint8 UPS_INVALID=3
	uint8 LOW_VOLTAGE=4
	uint8 RANGE_SENSOR_UNAVAILABLE=5
	uint8 RANGE_SENSOR_INVALID=6
	uint8 OBSTACLE_TOO_CLOSE=7
	uint8 INTERNAL_ERROR=8
	builtin_interfaces/Time stamp
		int32 sec
		uint32 nanosec
	uint8 command
	bool accepted
	uint8 reason
	string detail
safety_interfaces/ArmState state
	uint8 STANDBY=0
	uint8 ARMED=1
	builtin_interfaces/Time stamp
		int32 sec
		uint32 nanosec
	uint8 state
	uint32 revision
</pre>
