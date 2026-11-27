from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node, PushROSNamespace
from launch_ros.substitutions import FindPackageShare
from robot_bringup.utils.launch_utils import get_abs_name, LaunchDefaults


def generate_launch_description():
    
    default_params_file = PathJoinSubstitution([
        FindPackageShare("robot_bringup"),
        LaunchDefaults.config_dir,
        "actions.yaml"
    ])

    params = LaunchConfiguration("params")
    namespace = LaunchConfiguration("namespace")

    arming_atate_topic = get_abs_name(namespace, "/system/state/arming_state")
    blue_led_topic = get_abs_name(namespace, "/hardware/gpio/blue_led/set")
    short_press_adapter_topic = get_abs_name(namespace, "/interfaces/adapters/button/short_press")

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
                PushROSNamespace("actions/blink"),

                Node(
                    package="blink_control",
                    executable="blink_action_server",
                    name="action_server",
                    remappings=[
                        ("arming_state", arming_atate_topic),
                        ("set_blue_led", blue_led_topic)
                    ],
                    parameters=[
                        params
                    ],
                    output="screen"
                ),
        
                Node(
                    package="blink_control",
                    executable="blink_action_client",
                    name="action_client",
                    remappings=[
                        ("trigger_topic", short_press_adapter_topic)
                    ],
                    parameters=[
                        params
                    ],
                    output="screen"
                )
            ]
        )
        

    ])

    
    