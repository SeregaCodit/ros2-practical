from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import PathJoinSubstitution, LaunchConfiguration

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    default_config = PathJoinSubstitution([
        FindPackageShare("cpu_temperature_alarm"),
        "config",
        "cpu_temp_alarm.yaml"
    ])

    return LaunchDescription([
        DeclareLaunchArgument(
            "config_file",
            default_value=default_config,
            description="path to parameter configuration file"
        ),

        Node(
            package="cpu_temperature_alarm",
            executable="cpu_temperature_alarm_notificator",
            name="cpu_temperature_alarm_notificator",
            parameters=[
                LaunchConfiguration("config_file")
            ],
            output="screen")


    ])