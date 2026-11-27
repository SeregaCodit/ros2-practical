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
        "interfaces.yaml"
    ])

    remaps = GraphNames(namespace)

    return LaunchDescription([
        DeclareLaunchArgument(
            "params",
            default_value=default_params_file,
            description="Interfaces parameters YAML file"
        ),
        DeclareLaunchArgument(
            "namespace",
            default_value=LaunchDefaults.namespace,
            description="default root_namespace"
        ),

        GroupAction(
            actions=[
                PushROSNamespace(namespace),
                PushROSNamespace("interfaces"),

                Node(
                    package="button_long_press_adapter",
                    executable="button_long_press_adapter",
                    namespace="adapters/button",
                    name="long_press_adapter",
                    remappings=[
                        ("button_state_topic", remaps.button_state),
                        ("long_press", remaps.button_long_press)
                    ],
                    parameters=[params]
                ),

                Node(
                    package="button_short_press_adapter",
                    executable="button_short_press_adapter",
                    namespace="adapters/button",
                    name="short_press_adapter",
                    remappings=[
                        ("button_state_topic", remaps.button_state),
                        ("short_press", remaps.button_short_press)
                    ],
                    parameters=[params]
                ),

                Node(
                    package="led_arming_indication_controller",
                    executable="arming_indication_controller",
                    namespace="controllers",
                    name="led_arm_disarm_indication_controller",
                    remappings=[
                        ("set_green_led", remaps.set_green_led),
                        ("set_red_led", remaps.set_red_led),
                        ("state_publisher", remaps.arming_state)
                    ],
                    parameters=[params]
                )

            ]
        )
    ])