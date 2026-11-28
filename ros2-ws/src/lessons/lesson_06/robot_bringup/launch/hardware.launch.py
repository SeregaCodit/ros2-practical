from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node, PushROSNamespace
from launch_ros.substitutions import FindPackageShare
from robot_bringup.utils.launch_utils import LaunchDefaults, GraphNames


def generate_launch_description():
    params = LaunchConfiguration("params")
    namespace = LaunchConfiguration("namespace")

    default_params_file = PathJoinSubstitution([
        FindPackageShare("robot_bringup"),
        LaunchDefaults.config_dir,
        "hardware.yaml"
    ])

    remaps = GraphNames(namespace)

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
                        ("button_state", remaps.button_state)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="red_led_driver",
                    remappings=[
                        ("set_line", remaps.set_red_led)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="green_led_driver",
                    remappings=[
                        ("set_line", remaps.set_green_led)
                    ],
                    parameters=[params],
                    output="screen"
                ),

                Node(
                    package="gpio_driver",
                    executable="led_node",
                    name="blue_led_driver",
                    remappings=[
                        ("set_line", remaps.set_blue_led)
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
                        ("ups_battery_state", remaps.battery_state)
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
                        ("range_topic", remaps.tof_range)
                    ],
                    parameters=[params],
                    output="screen"
                )
            ]
        )


        
    ])

    