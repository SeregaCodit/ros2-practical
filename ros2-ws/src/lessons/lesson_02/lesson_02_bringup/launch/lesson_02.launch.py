from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.substitutions import PathJoinSubstitution, LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    alarm_launch = PythonLaunchDescriptionSource(
                    PathJoinSubstitution([
                        FindPackageShare("cpu_temperature_alarm"),
                        "launch",
                        "cpu_temp_alarm.launch.py"
                    ])
                )
    
    alarm_config_file = PathJoinSubstitution([
            FindPackageShare("cpu_temperature_alarm"),
            "config",
            "cpu_temp_alarm.yaml"
        ])

    return LaunchDescription([
        DeclareLaunchArgument(
            "alarm_config_file",
            default_value=alarm_config_file,
            description="config file for cpu temp alarm system"
        ),

        IncludeLaunchDescription(
            alarm_launch,
            launch_arguments={
                "config_file": LaunchConfiguration("alarm_config_file")
            }.items()
        ),
        
        Node(
            package="robot_monitor_py",
            executable="cpu_temperature_publisher_v2",
            name="cpu_temp_publisher",
            output="screen"
        ),

        Node(
            package="robot_monitor_cpp",
            executable="cpu_temperature_listener_v2",
            name="cpu_temp_listener",
            output="screen"
        )
    ])
