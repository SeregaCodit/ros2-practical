# WSL2 Mirrored Networking: затримка локального DDS-з'єднання

## Опис та контекст проблеми
**Статус:** <span style="color:orange">● Відоме обмеження, workaround прийнято</span>  
**Компоненти:** WSL2 / Docker / Fast DDS / ROS 2 Lyrical  
**Виявлено на:** Dev-PC (Windows 11 + WSL2 Ubuntu 26.04)

---

## 💻 Конфігурація середовища

| Компонент | Значення |
| :--- | :--- |
| **WSL** | 3.0.3.0 |
| **Linux Kernel** | 6.18.54.1-1 |
| **Windows Build** | 10.0.26200.9711 |
| **WSL Networking** | mirrored |
| **ROS 2** | Lyrical |
| **DDS** | Fast DDS (`rmw_fastrtps_cpp`) |
| **ROS Domain ID** | 42 |
| **Docker Network** | host |
| **Docker IPC** | host |

---

## 📌 Симптоми

Під час запуску `talker` та `listener` в одному Docker-контейнері на Dev-PC вузли не починають обмінюватися повідомленнями одразу:
```bash
ros2 run demo_nodes_cpp talker
ros2 run demo_nodes_cpp listener
```

Вузол `talker` публікує повідомлення раз на секунду, але локальний `listener` починає їх отримувати лише через **90–150+ секунд** або не отримує взагалі навіть після тривалого очікування.

Водночас обмін між різними фізичними пристроями (Dev-PC та Raspberry Pi) працює без будь-якої помітної затримки:

| Publisher | Subscriber | Результат |
| :--- | :--- | :--- |
| Raspberry Pi | Raspberry Pi | <span style="color:green">Миттєво</span> |
| Raspberry Pi | Dev-PC | <span style="color:green">Миттєво</span> |
| Dev-PC | Raspberry Pi | <span style="color:green">Миттєво</span> |
| **Dev-PC** | **Dev-PC** | ❌ **Затримка 90–150+ с** |

*Примітка: Команда `ros2 node list` може показувати обидва локальні вузли в ROS Graph, навіть коли самі повідомлення ще не надходять.*

---

## 🔍 Діагностика

Для пошуку аномалій було перевірено та виключено наступні фактори:
*   [x] Docker налаштовано в режимах `network_mode: host` та `ipc: host`.
*   [x] Перевірено збіг `ROS_DOMAIN_ID` та базових налаштувань DDS.
*   [x] UDP unicast через `127.0.0.1` — працює коректно.
*   [x] Базовий UDP multicast — працює.
*   [x] Виявлення `/talker` та `/listener` у ROS Graph — працює.
*   [x] Примусове використання UDP-only у Fast DDS — не усунуло затримку.
*   [x] Перезапуск контейнерів та WSL2 — не забезпечив стабільного результату.

### 🧪 Контрольний тест: Mirrored vs NAT
У файлі конфігурації Windows `%USERPROFILE%\.wslconfig` змінено режим мережі:
```ini
[wsl2]
# Було: networkingMode=mirrored
networkingMode=nat
```

Після цього виконано повне перезавантаження підсистеми в PowerShell:
```powershell
wsl --shutdown
```

**Результат A/B-тесту:**

| Режим WSL2 | Локальний DDS |
| :--- | :--- |
| `mirrored` | Затримка 90–150+ секунд |
| `nat` | <span style="color:green">Практично миттєво</span> |

Це експериментально підтверджує пряму залежність проблеми від мережевого режиму **WSL2 Mirrored Networking**.

---

## 📋 Імовірна причина

Особливості взаємодії **Fast DDS** із мережевим стеком **WSL2 Mirrored Networking**. 
Точну першопричину наразі не встановлено. Зокрема, не доведено, що проблема обмежується лише етапом *DDS Discovery* або механізмом *Shared Memory Transport*.

---

## <span style="color:green">💡 Прийняте рішення (Workaround)</span>

Для навчального проєкту `ros2-practical` затверджено такий підхід:
1.  **<span style="color:green">Залишаємо networkingMode=mirrored</span>**, оскільки він є критично необхідним для забезпечення прозорої мережевої взаємодії між хостом та зовнішньою Raspberry Pi.
2.  **Залишаємо Fast DDS** для збереження однорідності middleware між усіма пристроями та майбутньої інтеграції робототехнічних компонентів.
3.  **Тимчасово допускаємо затримку** локального DDS-з'єднання на Dev-PC під час тестування "сам на себе". Не покладаємося на швидке встановлення локального зв'язку в цьому середовищі.
4.  Для виконання критичних та чутливих до часу робототехнічних вузлів використовуємо Raspberry Pi або чисте Linux-середовище без цього обмеження.

⚠️ **Увага:** Затримка 90–150+ секунд **не вважається допустимою** поведінкою ROS 2 для реального автономного робота. Це виключно локальне компромісне обмеження конкретного інструментарію розробки.

---

## 🚀 Подальші дослідження

Якщо ця затримка почне критично заважати процесу розробки, заплановано наступні кроки:
*   Порівняти поведінку Fast DDS із **Cyclone DDS** (`rmw_cyclone_dds_cpp`).
*   Проаналізувати локальні RTPS-пакети за допомогою Wireshark.
*   Спробувати явно налаштувати DDS-мережеві інтерфейси (Network Interfaces Whitelist) в конфігураційному XML-файлі Fast DDS.

---

## 🔗 Корисні джерела
*   [Microsoft: WSL networking Documentation](https://microsoft.com)
*   [Microsoft WSL GitHub: обговорення ROS 2 та mirrored networking](https://github.com)
*   [eProsima Fast DDS Documentation](https://eprosima.com)
