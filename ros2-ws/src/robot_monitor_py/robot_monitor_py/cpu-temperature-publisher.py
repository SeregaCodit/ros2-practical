from pathlib import Path

import rclpy
from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from std_msgs.msg import Float32


TOPIC_NAME = '/robot/diagnostics/cpu_temperature'
TERMAL_PATH = Path('/sys/class/thermal/thermal_zone0/temp')


class CPUTemperaturePublisher(Node):
    def __init__(self) -> None:
        super().__init__('cpu_temperature_publisher')
        self.publisher_ = self.create_publisher(Float32, TOPIC_NAME, 10)
        self.timer = self.create_timer(1.0, self.publish_cpu_temperature)
        self.get_logger().info("CPU Temperature Publisher has been started.")

    def publish_cpu_temperature(self) -> None:
        try:
            raw_data = TERMAL_PATH.read_text().strip()
            temperature_celsius = float(raw_data) / 1000.0
        except (OSError, ValueError) as e:
            self.get_logger().error(f"Sensor read error: {e}")
            return

        message = Float32()
        message.data = temperature_celsius

        self.publisher_.publish(message)

        self.get_logger().info(f"CPU temperature: {temperature_celsius:.1f} C")


def main(args=None) -> None:
    try:
        with rclpy.init(args=args):
            node = CPUTemperaturePublisher()
            try:
                rclpy.spin(node)
            finally:
                node.destroy_node()
    except (KeyboardInterrupt, ExternalShutdownException):
        pass


if __name__ == '__main__':
    main()
