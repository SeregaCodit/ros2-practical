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

