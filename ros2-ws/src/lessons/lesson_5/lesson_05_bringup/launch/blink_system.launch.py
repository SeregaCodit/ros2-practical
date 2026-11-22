from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    short_press_period = LaunchConfiguration("max_push_period_ms")
    blue_led_line = LaunchConfiguration("blue_led_line")


    return LaunchDescription([
        DeclareLaunchArgument(
            "max_push_period_ms", default_value="500"
        ),
        DeclareLaunchArgument(
            "blue_led_line", default_value="23"
        ),

        Node(
            package="button_short_press_adapter",
            executable="button_short_press_adapter",
            name="button_short_press_adapter",
            remappings=[
                ("/button_state_topic", "/hardware/button/pressed")
            ],
            parameters=[{
                "max_push_period_ms": ParameterValue(short_press_period, value_type=int)
            }]
        ),
        Node(
            package="gpio_driver",
            executable="led_node",
            name="blue_led_node",
            remappings=[
                ("/hardware/led/set", "/set_gpio23")
            ],
            parameters=[{
                "line": ParameterValue(blue_led_line, value_type=int)
            }],
            output="screen"
        ),
        Node(
            package="blink_control",
            executable="blink_action_server",
            name="blink_action_server",
            parameters=[{
                "gpio_line": ParameterValue(blue_led_line, value_type=int)
            }]
        )
    ])