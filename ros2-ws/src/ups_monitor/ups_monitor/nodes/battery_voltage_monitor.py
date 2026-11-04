import rclpy

from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from sensor_msgs.msg import BatteryState

from ..core.interfaces.ibattery import IBattery
from ..infrastructure.liion_18650 import LiIon18650


TOPIC_NAME = "/robot/diagnostics/ups_battery_state"


class BatteryVoltageMonitor(Node):
    def __init__(self, battery: IBattery):
        super().__init__("ups_battery_voltage_monitor")

        self.__battery = battery
        self.__prev_status_val: int = -1

        self.suscription = self.create_subscription(BatteryState, TOPIC_NAME, self.callback, 10)
        self.get_logger().info("INITIALIZED")


    def callback(self, msg: BatteryState) -> None:
        voltage = msg.voltage
        status = self.__battery.voltage_status(voltage)

        log_text = f"Battery Voltage is {status.name}: {voltage} V"

        if status.value != self.__prev_status_val:
            if status.value > 0:
                self.get_logger().warning(log_text)
            else:
                self.get_logger().info(log_text)

            self.__prev_status_val = status.value


def main(args=None):
    battery = LiIon18650(cell_count=2)

    try:
        with rclpy.init(args=args):
            node = BatteryVoltageMonitor(battery)

            try:
                rclpy.spin(node)
            finally:
                node.destroy_node()
    except (KeyboardInterrupt, ExternalShutdownException):
        pass



