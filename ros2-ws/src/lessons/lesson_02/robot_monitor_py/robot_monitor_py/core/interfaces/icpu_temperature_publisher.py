from abc import ABC, abstractmethod


class ICpuTemperaturePublisher(ABC):
    @abstractmethod
    def publish_cpu_temperature(self) -> None:
        """read and publish cpu temperature"""
        pass