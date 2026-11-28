from launch import Substitution
from typing import Final

class LaunchDefaults:
    namespace: Final[str] = "robot_01"
    config_dir: Final[str] = "config"


def get_abs_name(namespace: str, name: str) -> list[str]:
    return[
        "/",
        namespace,
        "/",
        name.lstrip("/")
    ]


class GraphNames:
    def __init__(self, namespace: str | Substitution):
        #  ====== HARDWARE ======
        self.button_state = get_abs_name(
            namespace, "hardware/button/pressed"
        )

        self.battery_state = get_abs_name(
            namespace, "hardware/sensors/battery"
        )

        self.tof_range = get_abs_name(
            namespace, "hardware/sensors/tof_sensor"
        )

        self.set_red_led = get_abs_name(
            namespace, "hardware/gpio/red_led/set"
        )

        self.set_green_led = get_abs_name(
            namespace, "hardware/gpio/green_led/set"
        )

        self.set_blue_led = get_abs_name(
            namespace, "hardware/gpio/blue_led/set"
        )

        # ====== INTERFACES ======
        self.button_long_press = get_abs_name(
            namespace, "interfaces/adapters/button/long_press"
        )

        self.button_short_press = get_abs_name(
            namespace, "interfaces/adapters/button/short_press"
        )

        # ====== SYSTEM STATE ======
        self.arming_state = get_abs_name(
            namespace, "system/state/arming_state"
        )
        

    
