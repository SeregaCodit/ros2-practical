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
        "safety.yaml"
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
                PushROSNamespace("safety/arm_disarm"),
       

            Node(
                package="arming_manager",
                executable="arm_disarm_manager",
                name="arming_manager",
                remappings=[
                    ("battery_state_topic", remaps.battery_state),
                    ("range_sensor_topic", remaps.tof_range)
                ],
                parameters=[params],
                output="screen",
            ),

            Node(
                package="arm_controller",
                executable="arm_controller",
                name="arm_controller",
                remappings=[
                    ("toggle_state", remaps.button_long_press),
                    ("arming_state", remaps.arming_state)
                ],
                parameters=[params],
                output="screen",
            )
            ]
        ),
    ])