import math

from smbus2 import SMBus
from sensor_msgs.msg import BatteryState

from ..core.interfaces.ibattery_reader import IBatteryStatus


class UpsHatBIna219(IBatteryStatus):
    # INA219 datasheet info
    NAME: str = "UPS HAT(B)" 

    BUS_VOLTAGE_REG: int = 0x02
    SHUNT_VOLTAGE_REG: int = 0x01

    DATA_LENGTH: int = 2
    BYTE: int = 8
    BUS_VOLTAGE_SHIFT: int = 3

    BUS_VOLTAGE_LSB: float = 0.004      # V/bit
    SHUNT_VOLTAGE_LSB: float = 0.00001  # V/bit

    R_SHUNT: float = 0.1 # Ohms

    # UPS HAT(B) works with 2 18650 li-ion batteries
    BAT_COUNT: int = 2                  
    # 18650 default General values
    BAT_MAX_VOLTAGE: float = 4.2
    BAT_NOMINAL_VOLTAGE: float = 3.7
    # if cell voltage is lower than that value, we can 
    # think that this cell is dead an we can't use it anymore
    BAT_DEEP_DISCARGE_VOLTAGE: float = 2.75  


    def __init__(self, bus: SMBus, address: int = 0x42, battery_present: bool = True):
        self._bus = bus
        self._address = address
        self._battery_present = battery_present # UPS HAT(B) doesn't have automatic battery detect sysyem


    def _read_register(self, register: int) -> int:
        data = self._bus.read_i2c_block_data(
            self._address,
            register,
            self.DATA_LENGTH
        )

        return (data[0] << self.BYTE) | data[1]
    

    def voltage(self) -> float:
        """ read bus voltage in volts """
        raw = self._read_register(self.BUS_VOLTAGE_REG)
        voltage = (raw >> self.BUS_VOLTAGE_SHIFT) * self.BUS_VOLTAGE_LSB
        return voltage
    

    def current(self) -> float:
        """
        Read battery current in amperes.

        Positive: charging.
        Negative: discharging.

        Uses the INA219 shunt voltage register
        and the UPS HAT (B) 0.1 Ohm shunt.
        """
        raw = self._read_register(self.SHUNT_VOLTAGE_REG)

        if raw & 0x8000:
            raw -= 0x10000

        shunt_voltage = raw * self.SHUNT_VOLTAGE_LSB
        current = shunt_voltage / self.R_SHUNT
        return current


    def temperature(self) -> float:
        # Temperature in Degrees Celsius (If unmeasured NaN)
        #
        return math.nan
    

    def charge(self) -> float:
    # Current charge in Ah  (If unmeasured NaN)
        return math.nan


    def capacity(self) -> float:
    # Capacity in Ah (last full capacity)  (If unmeasured NaN)
        return math.nan


    def design_capacity(self) -> float:
        # Capacity in Ah (design capacity)  (If unmeasured NaN)
        return math.nan


    def percentage(self) -> float:
        # Charge percentage on 0 to 1 range  (If unmeasured NaN)
        return math.nan


    def power_supply_status(self) -> int:
        # The charging status as reported. Values defined above
        deadband = 0.005
        current = self.current()
        if not math.isfinite(current):
            return BatteryState.POWER_SUPPLY_STATUS_UNKNOWN

        if current > deadband:
            return BatteryState.POWER_SUPPLY_STATUS_CHARGING

        if current < -deadband:
            return BatteryState.POWER_SUPPLY_STATUS_DISCHARGING

        return BatteryState.POWER_SUPPLY_STATUS_NOT_CHARGING
        


    def power_supply_health(self) -> int:
        # The battery health metric. Values defined above
        # UPS HAT(B) can't measure each separete battery so we cant't
        # say the power supply is good if sum of batteries voltage value
        # is between deep discharge and overcharge values
        
        deadband = 0.05
        voltage = self.voltage()

        if not math.isfinite(voltage) or voltage <= 0:
            return BatteryState.POWER_SUPPLY_HEALTH_UNKNOWN

        max_voltage = self.BAT_MAX_VOLTAGE * self.BAT_COUNT
        deep_discharge = (
            self.BAT_DEEP_DISCARGE_VOLTAGE * self.BAT_COUNT
        )


        if voltage > (max_voltage + deadband):
            return BatteryState.POWER_SUPPLY_HEALTH_OVERVOLTAGE

        if voltage < deep_discharge:
            return BatteryState.POWER_SUPPLY_HEALTH_DEAD

        return BatteryState.POWER_SUPPLY_HEALTH_UNKNOWN


    def power_supply_technology(self) -> int:
        # The battery chemistry. Values defined above
        # UPS HAT_B works from 18650 Li-ion batteries
        return BatteryState.POWER_SUPPLY_TECHNOLOGY_LION


    def present(self) -> bool:
        # True if the battery is present
        return self._battery_present


    def cell_voltage(self) -> list[float]:
        # An array of individual cell voltages for each cell in the pack
        # If individual voltages unknown but number of cells known set each to NaN

        # ups_hat_b works from 2 cells
        return [math.nan, math.nan]


    def cell_temperature(self) -> list[float]:
        # An array of individual cell temperatures for each cell in the pack
        # If individual temperatures unknown but number of cells known set each to NaN
        return [math.nan, math.nan]


    def location(self) -> str:
        # The location into which the battery is inserted. (slot number or plug)
        return self.NAME


    def serial_number(self) -> str:
        # The best approximation of the battery serial number
        return ""
    