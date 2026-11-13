from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package="tof_sensor",
            executable="tof_range_publisher",
            prefix="/opt/robot-venv/bin/python3",
            output="screen"
        )
    ])