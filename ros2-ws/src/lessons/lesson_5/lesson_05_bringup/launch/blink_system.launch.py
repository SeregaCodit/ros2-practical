from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    short_press_period = LaunchConfiguration("max_push_period_ms")

    return LaunchDescription([
        DeclareLaunchArgument(
            "max_push_period_ms", default_value="500"
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
        )
    ])