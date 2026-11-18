# Breadboard
add blue LED on `GPIO23`
![lesson5.2_bb](../attachments/images/lesson5.2_bb.png)

# Check connections
full check of `GPIO` pins and `I2C ToF` sensor connection is in lesson 4: [click](lesson04_led_buttons_gpio_physical_nodes.md)

We have just connected a blue LED to GPIO23,  as shown in the breadbord circuit. Check it:

```bash
gpioset -c gpiochip0 23=1 # The blue LED is lit 
^C
gpioset -c gpiochip0 23=0 # The blue LED is not lit
^C

```


# Architecture
![lesson_5.2.drawio](../attachments/images/lesson_5.2.drawio.png)

# Interfaces

Since this lesson is a continuation of Lesson 5.1, we will add the `action` interface to the `lesson5.1/interfaces/system_interfaces` package to avoid duplicating packages that are semantically identical.

Build. Install. Check:
```bash
ros2 interface list | grep system_interfaces
```


````bash
# system_interfaces/msg/ArmingState
# system_interfaces/action/BlinkLed
````

```bash
ros2 interface show system_interfaces/action/BlinkLed 
```
```bash
# GOAL
# one cycle is OFF->ON state change sequence
uint32 cycles
uint32 half_preiod_ms
---
# RESULT
uint32 complited_cycles
---
# FEEDBACK
uint32 complited_cycles
bool led_on
```

# ButtonShortPressAdapter

Create a package in the adapters directory. Also implement **blink_system launch** and add the adapter node. Build. Source. Check:

```bash
ros2 launch lesson_05_bringup blink_system.launch.py
```

```bash
[INFO] [launch]: All log files can be found below /home/ros/.ros/log/2026-11-18-14-15-01-516081-ros2-pi-154233
[INFO] [launch]: Default logging verbosity is set to INFO
[INFO] [button_short_press_adapter-1]: process started with pid [154283]
[button_short_press_adapter-1] [INFO] [1795011302.278247165] [button_short_press_adapter]: INITIALIZED

# short press <= 500 ms
[button_short_press_adapter-1] [INFO] [1795011307.907340119] [button_short_press_adapter]: Button is pressed
[button_short_press_adapter-1] [INFO] [1795011308.006953562] [button_short_press_adapter]: button is realised. Ellapsed 0.099612 seconds
[button_short_press_adapter-1] [INFO] [1795011308.007453653] [button_short_press_adapter]: Publishing short press
# not short press > 500 ms
[button_short_press_adapter-1] [INFO] [1795011310.707047472] [button_short_press_adapter]: Button is pressed
[button_short_press_adapter-1] [INFO] [1795011312.206969256] [button_short_press_adapter]: button is realised. Ellapsed 1.499919 seconds
# long press > 3000 ms
[button_short_press_adapter-1] [INFO] [1795011318.306894147] [button_short_press_adapter]: Button is pressed
[button_short_press_adapter-1] [INFO] [1795011322.106890706] [button_short_press_adapter]: button is realised. Ellapsed 3.799995 seconds
# one more short press < 500 ms
[button_short_press_adapter-1] [INFO] [1795011327.807330886] [button_short_press_adapter]: Button is pressed
[button_short_press_adapter-1] [INFO] [1795011328.106926118] [button_short_press_adapter]: button is realised. Ellapsed 0.299594 seconds
[button_short_press_adapter-1] [INFO] [1795011328.107760800] [button_short_press_adapter]: Publishing short press
```

