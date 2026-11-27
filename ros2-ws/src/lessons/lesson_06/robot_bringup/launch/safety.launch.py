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
        "safety.yaml"
    ])

    battery_state_topic = get_abs_name(namespace, "hardware/sensors/battery")
    range_sensor_topic = get_abs_name(namespace, "hardware/sensors/tof_sensor")
    toggle_arm_disarm_topic = get_abs_name(namespace, "/interfaces/adapters/button/long_press")
    arming_atate_topic = get_abs_name(namespace, "/system/state/arming_state")
    arm_disarm_service = "arm_disarm_service"

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
                PushROSNamespace("safety/arm_disarm"),
       

            Node(
                package="arming_manager",
                executable="arm_disarm_manager",
                name="arming_manager",
                remappings=[
                    ("battery_state_topic", battery_state_topic),
                    ("range_sensor_topic", range_sensor_topic),
                    ("arm_disarm_manager", arm_disarm_service)
                ],
                parameters=[params],
                output="screen",
            ),

            Node(
                package="arm_controller",
                executable="arm_controller",
                name="arm_controller",
                remappings=[
                    ("arm_disarm_manager", arm_disarm_service),
                    ("toggle_state", toggle_arm_disarm_topic),
                    ("arming_state", arming_atate_topic)
                ],
                parameters=[params],
                output="screen",
            )
            ]
        ),
    ])