from smbus2 import SMBus
import logging


LOGGER = logging.getLogger(__name__)


class INA219_reader():
    # INA219 datashit info
    BUS_VOLTAGE_REG: int = 0x02
    SHUNT_VOLTAGE_REG: int = 0x01

    DATA_LENGTH: int = 2
    BYTE: int = 8
    BUS_VOLTAGE_SHIFT: int = 3

    BUS_VOLTAGE_LSB: float = 0.004      # V/bit
    SHUNT_VOLTAGE_LSB: float = 0.00001  # V/bit

    R_SHUNT: float = 0.1 # Ohms


    def __init__(self, bus: SMBus, address: int = 0x42):
        self._bus = bus
        self._address = address


    def _read_register(self, register: int) -> int:
        data = self._bus.read_i2c_block_data(
            self._address,
            register,
            self.DATA_LENGTH
        )

        return (data[0] << self.BYTE) | data[1]
    

    def get_voltage(self) -> float | None:
        """ read bus voltage in volts """
        raw = self._read_register(self.BUS_VOLTAGE_REG)
        voltage = (raw >> self.BUS_VOLTAGE_SHIFT) * self.BUS_VOLTAGE_LSB
        return voltage
    

    def get_current(self) -> float:
        raw = self._read_register(self.SHUNT_VOLTAGE_REG)

        if raw & 0x8000:
            raw -= 0x10000

        shunt_voltage = raw * self.SHUNT_VOLTAGE_LSB
        current = shunt_voltage / self.R_SHUNT
        return current
