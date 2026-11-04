from setuptools import find_packages, setup

package_name = 'ups_monitor'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    package_data={'': ['py.typed']},
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Serhii Naumenko',
    maintainer_email='serega.codit@gmail.com',
    description='TODO: Package description',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            "ups_battery_state_publisher = ups_monitor.nodes.battery_publisher:main",
            "battery_voltage_monitor = ups_monitor.nodes.battery_voltage_monitor:main"
        ],
    },
)
