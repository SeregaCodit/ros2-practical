from abc import ABC, abstractmethod

class IBatteryStatus(ABC):

    @abstractmethod
    def voltage(self) -> float:
        # Voltage in Volts (Mandatory)
        pass

    @abstractmethod
    def temperature(self) -> float:
        # Temperature in Degrees Celsius (If unmeasured NaN)
        pass

    @abstractmethod
    def current(self) -> float:
        # Negative when discharging (A)  (If unmeasured NaN)
        pass

    @abstractmethod
    def charge(self) -> float:
        # Current charge in Ah  (If unmeasured NaN)
        pass

    @abstractmethod
    def capacity(self) -> float:
        # Capacity in Ah (last full capacity)  (If unmeasured NaN)
        pass

    @abstractmethod
    def design_capacity(self) -> float:
        # Capacity in Ah (design capacity)  (If unmeasured NaN)
        pass

    @abstractmethod
    def percentage(self) -> float:
        # Charge percentage on 0 to 1 range  (If unmeasured NaN)
        pass

    @abstractmethod
    def power_supply_status(self) -> int:
        # The charging status as reported. Values defined above
        pass

    @abstractmethod
    def power_supply_health(self) -> int:
        # The battery health metric. Values defined above
        pass

    @abstractmethod
    def power_supply_technology(self) -> int:
        # The battery chemistry. Values defined above
        pass

    @abstractmethod
    def present(self) -> bool:
        # True if the battery is present
        pass

    @abstractmethod
    def cell_voltage(self) -> list[float]:
        # An array of individual cell voltages for each cell in the pack
        # If individual voltages unknown but number of cells known set each to NaN
        pass

    @abstractmethod
    def cell_temperature(self) -> int:
        # An array of individual cell temperatures for each cell in the pack
        # If individual temperatures unknown but number of cells known set each to NaN
        pass

    @abstractmethod
    def location(self) -> int:
        # The location into which the battery is inserted. (slot number or plug)
        pass

    @abstractmethod
    def serial_number(self) -> int:
        # The best approximation of the battery serial number
        pass