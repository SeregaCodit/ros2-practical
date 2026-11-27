from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node, PushROSNamespace
from launch_ros.substitutions import FindPackageShare

from robot_bringup.utils.launch_utils import get_abs_name, LaunchDefaults

def generate_launch_description():
    params = LaunchConfiguration("params")
    namespace = LaunchConfiguration("namespace")

    default_params_file = PathJoinSubstitution([
        FindPackageShare("robot_bringup"),
        LaunchDefaults.config_dir,
        "interfaces.yaml"
    ])

    button_state_topic = get_abs_name(namespace, "hardware/button/pressed")
    long_press_adapter_topic = get_abs_name(namespace, "/interfaces/adapters/button/long_press")
    short_press_adapter_topic = get_abs_name(namespace, "/interfaces/adapters/button/short_press")
    red_led_topic = get_abs_name(namespace, "hardware/gpio/red_led/set")
    green_led_topic = get_abs_name(namespace, "hardware/gpio/green_led/set")
    arming_atate_topic = get_abs_name(namespace, "/system/state/arming_state")

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
                        ("button_state_topic", button_state_topic),
                        ("long_press", long_press_adapter_topic)
                    ],
                    parameters=[params]
                ),

                Node(
                    package="button_short_press_adapter",
                    executable="button_short_press_adapter",
                    namespace="adapters/button",
                    name="short_press_adapter",
                    remappings=[
                        ("button_state_topic", button_state_topic),
                        ("short_press", short_press_adapter_topic)
                    ],
                    parameters=[params]
                ),

                Node(
                    package="led_arming_indication_controller",
                    executable="arming_indication_controller",
                    namespace="controllers",
                    name="led_arm_disarm_indication_controller",
                    remappings=[
                        ("set_green_led", green_led_topic),
                        ("set_red_led", red_led_topic),
                        ("state_publisher", arming_atate_topic)
                    ],
                    parameters=[params]
                )

            ]
        )
    ])