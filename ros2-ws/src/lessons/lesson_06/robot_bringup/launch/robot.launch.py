from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.substitutions import FindPackageShare

from robot_bringup.utils.launch_utils import LaunchDefaults

def generate_launch_description():
    robot_ns = LaunchConfiguration("robot_ns")
    
    actions_params = LaunchConfiguration("actions_params")
    hardware_params = LaunchConfiguration("hardware_params")
    interfaces_params = LaunchConfiguration("interfaces_params")
    safety_params = LaunchConfiguration("safety_params")

    package_share = FindPackageShare("robot_bringup")

    launch_dir = PathJoinSubstitution([package_share, "launch"])

    def get_default_params(filename: str) -> PathJoinSubstitution:
        return PathJoinSubstitution([
            package_share,
            LaunchDefaults.config_dir,
            filename
        ])

    def include_launch(filename: str, params: LaunchConfiguration) -> GroupAction:
        # use group action scoped=true for insure subsustems params are isolated
        return GroupAction(
            scoped=True,
            actions=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        PathJoinSubstitution([launch_dir, filename]),
                    ),
                    launch_arguments={
                        "params": params,
                        "namespace": robot_ns
                    }.items()
                )
            ]
        )
    
        

    return LaunchDescription([
        DeclareLaunchArgument(
            "robot_ns",
            default_value=LaunchDefaults.namespace,
            description="robot namespace"
        ),
        DeclareLaunchArgument(
            "actions_params",
            default_value=get_default_params("actions.yaml"),
            description="Params configuration for actions subsystem"
        ),
        DeclareLaunchArgument(
            "hardware_params",
            default_value=get_default_params("hardware.yaml"),
            description="Params configuration for hardware subsystem"
        ),
        DeclareLaunchArgument(
            "interfaces_params",
            default_value=get_default_params("interfaces.yaml"),
            description="Params configuration for interfaces subsystem"
        ),
        DeclareLaunchArgument(
            "safety_params",
            default_value=get_default_params("safety.yaml"),
            description="Params configuration for safety subsystem"
        ),

        include_launch("hardware.launch.py", params=hardware_params),
        include_launch("safety.launch.py", params=safety_params),
        include_launch("interfaces.launch.py", params=interfaces_params),
        include_launch("actions.launch.py", params=actions_params)

    ])