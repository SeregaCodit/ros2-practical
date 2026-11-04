from ..core.interfaces.ibattery import IBattery
from ..core.entities.enums import BatteryChargeStatus

from sensor_msgs.msg import BatteryState


class LiIon18650(IBattery):

    TYPE: int = BatteryState.POWER_SUPPLY_TECHNOLOGY_LION
    
    def __init__(
            self, 
            max_voltage: float = 4.2,
            nominal_voltage: float = 3.7,
            min_safe_voltage: float = 3.2,
            deep_discharge_voltage: float = 2.75,
            cell_count: int = 1
            ):

        self._max_voltage = max_voltage
        self._nominal_voltage = nominal_voltage
        self._min_safe_voltage = min_safe_voltage
        self._deep_discharge_voltage = deep_discharge_voltage
        self._cell_count = cell_count

    def voltage_status(self, voltage: float) -> str:
        if voltage > self.max_voltage * self.cell_count:
            return BatteryChargeStatus.HIGH
        elif voltage > self.min_safe_voltage * self.cell_count:
            return BatteryChargeStatus.NORMAL
        elif voltage > self.deep_discharge_voltage * 2:
            return BatteryChargeStatus.LOW
        return BatteryChargeStatus.DEEP_DISCHARGE


    @property
    def type(self) -> str:
        return self.TYPE

    
    @property
    def cell_count(self) -> int:
        return self._cell_count

    
    @cell_count.setter
    def cell_count(self, value: int):
        try:
            self._cell_count = int(value)
        except ValueError as ex:
            raise ex

    
    @property
    def max_voltage(self) -> float:
        return self._max_voltage
    

    @max_voltage.setter
    def max_voltage(self, value: float) -> None:
        try:
            self._max_voltage = float(value)
        except ValueError as ex:
            raise ex

    
    @property
    def nominal_voltage(self) -> float:
        return self._nominal_voltage
    

    @nominal_voltage.setter
    def nominal_voltage(self, value: float) -> None:
        try:
            self._nominal_voltage = float(value)
        except ValueError as ex:
            raise ex

    
    @property
    def min_safe_voltage(self) -> float:
        return self._min_safe_voltage
    

    @min_safe_voltage.setter
    def min_safe_voltage(self, value: float) -> None:
        try:
            self._min_safe_voltage = float(value)
        except ValueError as ex:
            raise ex
        

    @property
    def deep_discharge_voltage(self) -> float:
        return self._deep_discharge_voltage
    

    @deep_discharge_voltage.setter
    def deep_discharge_voltage(self, value: float) -> None:
        try:
            self._deep_discharge_voltage = float(value)
        except ValueError as ex:
            raise ex