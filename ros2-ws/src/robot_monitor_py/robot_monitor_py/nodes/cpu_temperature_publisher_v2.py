from pathlib import Path

import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from sensor_msgs.msg import Temperature

import time

from ..core.interfaces.icpu_temperature_publisher import ICpuTemperaturePublisher

TOPIC_NAME = "/robot/diagnostics/cpu_temperature"
TERMAL_PATH = Path('/sys/class/thermal/thermal_zone0/temp')
FRAME_ID = "base_link"


class CpuTemperaturePublisherV2(Node, ICpuTemperaturePublisher):
    def __init__(self) -> None:
        super().__init__("cpu_temperature_publisher_v2")
        self.publisher_ = self.create_publisher(Temperature, TOPIC_NAME, 10)
        self.timer = self.create_timer(1.0, self.publish_cpu_temperature)
        self.get_logger().info("CPU Temperature Publisher has been started.")

    def publish_cpu_temperature(self) -> None:

        try:
            raw_data = TERMAL_PATH.read_text().strip()
            # time stamp sould be recoreded as close as possible to the mesurement result
            time_stamp = self.get_clock().now().to_msg()
            temperature_c = float(raw_data) / 1000.0
        except (OSError, ValueError) as ex:
            self.get_logger().error(f"Sensor read error : {ex}")
            return

        msg = Temperature()
        msg.header.stamp = time_stamp
        msg.header.frame_id = FRAME_ID
        msg.temperature = temperature_c
        msg.variance = 0.0 # This doen't mean that the vairiance is equal to zero. It meanst that the variance is unknown

        self.publisher_.publish(msg)
        self.get_logger().info(f"CPU temperature: {temperature_c:.1f} C")


def main(args=None):

    try:
        with rclpy.init(args=args):
            node = CpuTemperaturePublisherV2()
            try:
                rclpy.spin(node)
            finally:
                node.destroy_node()

    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    

if __name__ == "__main__":
    main()