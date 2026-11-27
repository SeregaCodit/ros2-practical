from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    short_press_period = LaunchConfiguration("max_push_period_ms")
    blue_led_line = LaunchConfiguration("blue_led_line")
    half_period_ms = LaunchConfiguration("half_period_ms")
    action_cycles = LaunchConfiguration("action_cycles")


    return LaunchDescription([
        DeclareLaunchArgument(
            "max_push_period_ms", default_value="500"
        ),
        DeclareLaunchArgument(
            "blue_led_line", default_value="23"
        ),
        DeclareLaunchArgument(
            "half_period_ms", default_value="250"
        ),
        DeclareLaunchArgument(
            "action_cycles", default_value="20"
        ),

        Node(
            package="button_short_press_adapter",
            executable="button_short_press_adapter",
            name="button_short_press_adapter",
            remappings=[
                ("/button_state_topic", "button_state")
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
                ("set_line", "set_blue_led")
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
        ),
        Node(
            package="blink_control",
            executable="blink_action_client",
            name="blink_action_client",
            remappings=[
                ("trigger_topic", "short_press")
            ],
            parameters=[{
                "cycles": ParameterValue(action_cycles, value_type=int),
                "half_period_ms": ParameterValue(half_period_ms, value_type=int)
            }]
        )
    ])