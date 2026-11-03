from smbus2 import SMBus
import logging


LOGGER = logging.getLogger(__name__)


class INA219_voltage_reader():
    # INA219 datashit info
    BUS_VOLTAGE_REG: int = 0x02
    DATA_LENGTH: int = 2
    BYTE: int = 8
    INITIAL_BITS: int = 3
    VOLTS_MULT = 0.004

    def __init__(self, bus: SMBus, address: int = 0x42):
        self._bus = bus
        self._address = address

    def get_voltage(self) -> float | None:
        data = self._bus.read_i2c_block_data(
            self._address,
            self.BUS_VOLTAGE_REG,
            self.DATA_LENGTH
        )

        raw = (data[0] << self.BYTE) | data[1]
        voltage = (raw >> self.INITIAL_BITS) * self.VOLTS_MULT
        return voltage
        

def main():
    import time
    I2C_BUS = 1

    bus = SMBus(I2C_BUS)

    ina219 = INA219_voltage_reader(bus)
    with bus as b:
        for i in range(50):
        
            try:
                voltage = ina219.get_voltage()
                print(f"Battery voltage: {voltage:.3f} V")
            except OSError as ex:
                print(f"I2C read error: {ex}")

            time.sleep(0.5)
            i+=1

    print("end")
main()
