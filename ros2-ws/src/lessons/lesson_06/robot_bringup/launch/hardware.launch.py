from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node, PushROSNamespace
from launch_ros.substitutions import FindPackageShare
from robot_bringup.utils.launch_utils import get_abs_name, LaunchDefaults


def generate_launch_description():
    params = LaunchConfiguration("params")
    namespace = LaunchConfiguration("namespace")

    default_params_file = PathJoinSubstitution([
        FindPackageShare("robot_bringup"),
        LaunchDefaults.config_dir,
        "hardware.yaml"
    ])

    button_state = get_abs_name(namespace, "hardware/button/pressed")
    red_line = get_abs_name(namespace, "/hardware/gpio/red_led/set")
    green_line = get_abs_name(namespace, "/hardware/gpio/green_led/set")
    blue_line = get_abs_name(namespace, "/hardware/gpio/blue_led/set")
    battery_state_topic = get_abs_name(namespace, "hardware/sensors/battery")
    range_sensor_topic = get_abs_name(namespace, "hardware/sensors/tof_sensor")

    return LaunchDescription([
        DeclareLaunchArgument(
            "params",
            default_value=default_params_file,
            description="Actions parameters YAML file"
        ),
        DeclareLaunchArgument(
            "namespace",
            default_value=LaunchDefaults.namespace,
            description="default root_namespace"
        ),

        GroupAction(
            actions=[
                PushROSNamespace(namespace),
                PushROSNamespace("hardware/gpio"),

                Node(
                    package="gpio_driver",
                    executable="button_node",
                    name="button_driver",
                    remappings=[
                        ("button_state", button_state)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="red_led_driver",
                    remappings=[
                        ("set_line", red_line)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="green_led_driver",
                    remappings=[
                        ("set_line", green_line)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="blue_led_driver",
                    remappings=[
                        ("set_line", blue_line)
                    ],
                    parameters=[params],
                    output="screen"
                )

            ]
        ),

        GroupAction(
            actions=[
                PushROSNamespace(namespace),
                PushROSNamespace("hardware/sensors"),

                Node(
                    package="ups_monitor",
                    executable="ups_battery_state_publisher",
                    name="battery",
                    remappings=[
                        ("ups_battery_state", battery_state_topic)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="tof_sensor",
                    executable="tof_range_publisher",
                    prefix="/opt/robot-venv/bin/python3",
                    name="tof_sensor",
                    remappings=[
                        ("range_topic", range_sensor_topic)
                    ],
                    parameters=[params],
                    output="screen"
                )
            ]
        )


        
    ])

    