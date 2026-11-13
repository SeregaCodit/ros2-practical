import rclpy
import math
import adafruit_vl53l0x

from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from sensor_msgs.msg import Range
from adafruit_extended_bus import ExtendedI2C


PUBLISH_RATE_HZ: str = "publish_rate_hz"
FRAME_ID: str = "frame_id"
TOPIC_NAME: str = "range_topic"


class VL53L0X(Node):
    MIN_RANGE_M: float = 0.003
    MAX_RANGE_M: float = 2.0
    RADIATION_TYPE: int = 1 # INFRARED from Range msg
    FOV_RAD = math.radians(25.0)

    def __init__(self):
        super().__init__("tof_raw_range")

        self.declare_parameter(PUBLISH_RATE_HZ, 10)
        self.declare_parameter(FRAME_ID, "tof_link")

        rate = self.get_parameter(PUBLISH_RATE_HZ).value

        if not math.isfinite(rate) or rate <= 0:
            raise ValueError(f"{PUBLISH_RATE_HZ} must be positive and finite. \
                             Got {rate}")

        self.frame_id = self.get_parameter(FRAME_ID).value


        self.i2c = ExtendedI2C(1)
        self.sensor = adafruit_vl53l0x.VL53L0X(self.i2c)
        self.failed_reads = 0

        self._publisher = self.create_publisher(
            Range,
            TOPIC_NAME,
            10
        )

        self._pub_timer = self.create_timer(
            1.0 / rate,
            self._publish_range
        )
        
        self.get_logger().info(f"initialized" )


    def _publish_range(self):
        try:
            distance_mm = self.sensor.distance
        except Exception as ex:
            self.get_logger().error(f"read failed:\n{ex}")

        stamp = self.get_clock().now().to_msg()
        distance_m = distance_mm / 1000.0

        if (any([
            not math.isfinite(distance_m),
            not (self.MIN_RANGE_M <= distance_m <= self.MAX_RANGE_M)
        ])):
            self.report_failure(f"Invalid range: {distance_m} m")

        self.failed_reads = 0

        msg = Range()

        msg.header.stamp = stamp
        msg.header.frame_id = self.frame_id
        msg.radiation_type = self.RADIATION_TYPE
        msg.field_of_view = self.FOV_RAD
        msg.min_range = self.MIN_RANGE_M
        msg.max_range = self.MAX_RANGE_M
        msg.range = distance_m
        msg.variance = 0.0

        self._publisher.publish(msg)
    

    def report_failure(self, msg: str) -> None:
        self.failed_reads += 1

        if any([
            self.failed_reads == 1,
            self.failed_reads % 10 == 0
        ]):
            self.get_logger().warning(msg)


def main(args=None):

    try:
        with rclpy.init(args=args):
            node = VL53L0X()

            try:
                rclpy.spin(node)
            finally:
                node.destroy_node()

    except (KeyboardInterrupt, ExternalShutdownException):
        pass


if __name__ == "__main__":
    main()