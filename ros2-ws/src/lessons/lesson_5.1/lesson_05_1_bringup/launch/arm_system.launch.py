from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():
    button_line = LaunchConfiguration("button_line")
    green_led_line = LaunchConfiguration("green_led_line")
    red_led_line = LaunchConfiguration("red_led_line")

    return LaunchDescription([

        DeclareLaunchArgument(
            'button_line', default_value='17'
        ),
        DeclareLaunchArgument(
            'green_led_line', default_value='22'
        ),
        DeclareLaunchArgument(
            'red_led_line', default_value='27'
        ),
        # HARDWARE
        Node(
            package="gpio_driver",
            executable="button_node",
            name="button_node",
            parameters=[{
                "line": ParameterValue(button_line, value_type=int)
            }],
            output="screen"
        ),

        Node(
            package="gpio_driver",
            executable="led_node",
            name="red_led_node",
            remappings =[
                ("/hardware/led/set", "/set_gpio27")
            ],
            parameters=[{
                "line": ParameterValue(red_led_line, value_type=int)
            }],
            output="screen"
        ),

        Node(
            package="gpio_driver",
            executable="led_node",
            name="green_led_node",
            remappings =[
                ("/hardware/led/set", "/set_gpio22")
            ],
            parameters=[{
                "line": ParameterValue(green_led_line, value_type=int)
            }],
            output="screen"
                ),

        Node(
            package="ups_monitor",
            executable="ups_battery_state_publisher",
            name="ups_battery_state_publisher",
            output="screen"
        ),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([
                    FindPackageShare("tof_sensor"),
                    "launch",
                    "tof_sensor.launch.py"
                ])
            )
        ),
        # ADAPTERS
        Node(
            package="button_long_press_adapter",
            executable="button_long_press_adapter",
            name="button_long_press_adapter",
            remappings=[
                ("/button_state_topic", "/hardware/button/pressed")
            ],
            output="screen"
        ),
        # CONTROLLERS
        Node(
            package="arming_manager",
            executable="arm_disarm_manager",
            name="arm_disarm_manager",
            remappings=[
                ("/battery_state_topic", "/robot/diagnostics/ups_battery_state"),
                ("range_sensor_topic", "/range_topic")
            ],
            parameters=[{
                "heartbeat_timeout_ms": 2000
            }],
            output="screen"
        ),

        Node(
            package="arm_controller",
            executable="arm_controller",
            name="arm_controller",
            remappings=[
                ("/toggle_state", "/long_press")
            ],
            output="screen"
        ),

        Node(
            package="led_arming_indication_controller",
            executable="arming_indication_controller",
            name="arming_indication_controller",
            remappings=[
                ("/state_publisher", "/arming_state")
            ],
            output="screen"
        ),

    ])