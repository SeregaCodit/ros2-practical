import rclpy

from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from sensor_msgs.msg import BatteryState

from ups_monitor.core.interfaces.ibattery_reader import IBatteryStatus
from ..infrastructure.ups_hat_b_ina219 import UpsHatBIna219


TOPIC_NAME = "/robot/diagnostics/ups_battery_state"
FRAME_ID = "base_link"
I2C_BUS = 1


class BatteryStatePublisher(Node):
    def __init__(self, battery_reader: IBatteryStatus) -> None:
        super().__init__("ups_battery_state_publisher")

        self._battery_reader = battery_reader

        self._publisher = self.create_publisher(BatteryState, TOPIC_NAME, 10)
        self._timer = self.create_timer(1.0, self.callback)
        self.get_logger().info("INITIALIZED")


    def callback(self) -> None:
        msg = BatteryState()

        try:
            msg.header.stamp = self.get_clock().now().to_msg()
            msg.voltage = self._battery_reader.voltage()
            msg.current = self._battery_reader.current()
        except OSError as ex:
            err_msg = f"Failed to read UPS battery data: {ex}"
            self.get_logger().error(err_msg)
            raise OSError(err_msg)


        msg.header.frame_id = FRAME_ID
        msg.temperature = self._battery_reader.temperature()
        msg.charge = self._battery_reader.charge()
        msg.temperature = self._battery_reader.temperature()
        msg.capacity = self._battery_reader.capacity()
        msg.design_capacity = self._battery_reader.design_capacity()
        msg.percentage = self._battery_reader.percentage()
        msg.power_supply_status = self._battery_reader.power_supply_status()
        msg.power_supply_health = self._battery_reader.power_supply_health()
        msg.power_supply_technology = self._battery_reader.power_supply_technology()
        msg.present = self._battery_reader.present()
        msg.cell_voltage = self._battery_reader.cell_voltage()
        msg.cell_temperature = self._battery_reader.cell_temperature()
        msg.location = self._battery_reader.location()
        msg.serial_number = self._battery_reader.serial_number()

        self._publisher.publish(msg)

        self.get_logger().debug(str(msg))


def main(args=None):
    from smbus2 import SMBus

    with SMBus(I2C_BUS) as bus:
        ups_reader = UpsHatBIna219(bus)
        
        try:
            with rclpy.init(args=args):
                node = BatteryStatePublisher(ups_reader)
                
                try:
                    rclpy.spin(node)
                finally:
                    node.destroy_node()
        except (KeyboardInterrupt, ExternalShutdownException):
            pass
