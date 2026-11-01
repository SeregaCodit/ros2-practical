# local ping
* WSL 3.0.3.0, kernel 6.18.54.1-1.
* Fast DDS + WSL2 Mirrored: локальна **затримка 90–150+ секунд**.
* WSL2 NAT: локальне з'єднання встановлюється **миттєво**.
* Мережевий обмін LOQ ↔ Raspberry Pi у Mirrored працює.
* Причина всередині WSL2 Mirrored / Fast DDS ще не встановлена остаточно.
* Тимчасове рішення: залишаємо Fast DDS і Mirrored, не покладаємося на швидкий локальний discovery.