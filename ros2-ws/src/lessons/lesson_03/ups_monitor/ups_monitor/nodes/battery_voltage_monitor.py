import rclpy
import time
import math

from rclpy.node import Node
from rclpy.executors import ExternalShutdownException
from sensor_msgs.msg import BatteryState

from ..core.interfaces.ibattery import IBattery
from ..infrastructure.liion_18650 import LiIon18650

from ..infrastructure.Battery_voltage_monitor_params import battery_voltage_monitor


TOPIC_NAME = "ups_battery_state"


class BatteryVoltageMonitor(Node):
    def __init__(self, battery: IBattery):
        super().__init__("ups_battery_voltage_monitor")

        self._params_listener = battery_voltage_monitor.ParamListener(self)
        self._params = self._params_listener.get_params();

        if self._params.watchdog_period_s >= self._params.msg_timeout_s:
            raise ValueError("watchdog_period_s must be < msg_timeout_s" )

        self.__battery = battery
        self.__prev_status_val: int = -1
        
        # Watchdog state
        self._last_msg_time = time.monotonic()
        self._telemetry_timed_out = False
        self._msg_timeout_sec = self._params.msg_timeout_s

        self.suscription = self.create_subscription(BatteryState, TOPIC_NAME, self.callback, 10)

        self._watchdog_timer = self.create_timer(
            self._params.msg_timeout_s,
            self.__check_telemetry,
        )

        self.get_logger().info("INITIALIZED")


    def callback(self, msg: BatteryState) -> None:
        voltage = msg.voltage

        if math.isfinite(voltage) and voltage > 0.0:

            self._last_msg_time = time.monotonic()

            if self._telemetry_timed_out:
                self._telemetry_timed_out = False

                self.get_logger().info(
                    "UPS battery telemetry restored"
                )
        
        status = self.__battery.voltage_status(voltage)

        log_text = f"Battery Voltage is {status.name}: {voltage:.2f} V"

        if status.value != self.__prev_status_val:
            if status.value > 0:
                self.get_logger().warning(log_text)
            else:
                self.get_logger().info(log_text)

            self.__prev_status_val = status.value


    def __check_telemetry(self) -> None:
        elapsed = (time.monotonic() - self._last_msg_time)

        if (elapsed >= self._msg_timeout_sec
            and not self._telemetry_timed_out):

            self._telemetry_timed_out = True

            # Force re-evaluation after recovery
            self.__prev_status_val = -1

            self.get_logger().warning(
                "UPS battery telemetry timeout: "
                f"no valid voltage measurements for {elapsed:.1f} s"
            )


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



