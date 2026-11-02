# Docker: container exits with Exited (1) after a successful build

## Компонент: Docker / ROS 2 Lyrical / Ubuntu 26.04
**Виявлено на:** Ubuntu 20.04, проєкт на зовнішньому SSD із файловою системою NTFS.

**Статус:** <span style="color:green">● Resolved (Вирішено)</span>

---

## 📌 Симптоми

Команда для збірки та запуску:
```bash
docker compose up -d --build
```
Успішно збирає Docker image та повідомляє про запуск контейнера:
```text
✔ ros2 Built
✔ Container ros2_practical_dev_container Started
```

Але спроба підключитися через:
```bash
docker compose exec ros2 bash
```
Повертає помилку:
```text
service "ros2" is not running
```

### 🔍 Діагностика:
```bash
docker compose ps -a
docker compose logs --tail=100 ros2
```
**Результат:** Контейнер має статус `Exited (1)`.

---

## ❌ Проблема 1. Security violation під час запуску entrypoint

### У логах:
```text
Security violation: Requested utility `ros-entrypoint` does not match executable name:
  /usr/lib/cargo/bin/coreutils/env
```

### 📋 Причина
У файлі `entrypoint.sh` використовувався shebang:
```bash
#!/usr/bin/env bash
```
У Docker-образі на базі **Ubuntu 26.04** використовується реалізація `env` із **Rust Coreutils (uutils)**. У певних умовах її перевірка імені виконуваної утиліти призводить до помилки *Security violation*. 

*Примітка: Проблема виникає всередині Docker-образу, тому версія Ubuntu на хості (20.04) не є безпосередньою причиною.*

### <span style="color:green">💡 Рішення</span>
У файлі `entrypoint.sh` замінити shebang на прямий виклик інтерпретатора:
*   *Неправильно:* `#!/usr/bin/env bash`
*   *<span style="color:green">Правильно:</span>* `#!/bin/bash`

Таким чином Bash викликається безпосередньо, без проміжного `/usr/bin/env`.

---

## ❌ Проблема 2. Permission denied після виправлення shebang

Після першого виправлення контейнер усе ще завершувався з кодом 1.

### У логах:
```text
/bin/bash: /usr/local/bin/ros-entrypoint: Permission denied
```

### 📋 Причина
Користувач контейнера `ros` не мав необхідних прав доступу до entrypoint-скрипта. 

Проєкт розташований на **NTFS**, де POSIX-права можуть оброблятися інакше, ніж на ext4. Це могло вплинути на права вихідного файла на хості, але безпосередню причину відсутності прав саме через NTFS не було доведено.

### <span style="color:green">💡 Рішення</span>
Змінити інструкцію копіювання в `Dockerfile`. Замість послідовного копіювання та зміни прав через `RUN` (що могло успадковувати обмеження хоста):
```dockerfile
COPY entrypoint.sh /usr/local/bin/ros-entrypoint
RUN chmod +x /usr/local/bin/ros-entrypoint
```
Використовувати **явно задані права** безпосередньо під час копіювання:
```dockerfile
COPY --chmod=755 entrypoint.sh /usr/local/bin/ros-entrypoint
```
Це гарантовано встановлює права всередині Docker image незалежно від режиму доступу вихідного файла:
*   **Власник:** читання, запис, виконання.
*   **Група:** читання, виконання.
*   **Інші:** читання, виконання.

---

## <span style="color:green">🛠️ Остаточна робоча конфігурація</span>

### `entrypoint.sh`
```bash
#!/bin/bash
set -e

# Подальша ініціалізація ROS 2
```

### `Dockerfile`
```dockerfile
COPY --chmod=755 entrypoint.sh /usr/local/bin/ros-entrypoint

ENTRYPOINT ["/usr/local/bin/ros-entrypoint"]
```

---

## <span style="color:green">🚀 Застосування змін та перевірка</span>

1. Перезбірка із повним перезапуском з кореневого каталогу репозиторію:
   ```bash
   docker compose up -d --build --force-recreate ros2
   ```

2. Перевірка статусу та підключення:
   ```bash
   docker compose ps
   docker compose exec ros2 bash
   ```

3. Тестування оточення всередині контейнера:
   ```bash
   echo "\$ROS_DISTRO"
   ros2 node list
   ```

### <span style="color:green">✅ Очікуваний результат (Успішно)</span>
*   Контейнер має статус **`Up`**.
*   Команда `docker compose exec` успішно відкриває сесію.
*   Змінна оточення повертає `ROS_DISTRO=lyrical`.
*   Утиліти ROS 2 CLI запускаються без помилок.
*   **Результат:** Обидва виправлення застосовані на *Pivden PC*; Docker-контейнер успішно запускається.

---

## 📝 Примітки щодо переносимості

Виправлення повністю придатні та перевірені для Linux Docker-образів на:
*   **Ubuntu 20.04** (хост Pivden PC).
*   **WSL2 Ubuntu 26.04** (LOQ).
*   **Raspberry Pi** (ARM64 Linux).

Вони є універсальними й не залежать від архітектури CPU за умови наявності `/bin/bash` у контейнері.

⚠️ **Важливо:** Прапорець `COPY --chmod=755` виправляє права **лише** для скопійованого файла всередині образу. Це не усуває потенційні проблеми прав, символічних посилань (symlinks) та загальної продуктивності ROS 2 workspace, який монтується з NTFS через bind mount (`volumes`). 
*Для коректної роботи команди `colcon build --symlink-install` наполегливо рекомендується використовувати рідну файлову систему Linux (**ext4**).*

---

## 🔗 Корисні джерела
*   [Dockerfile COPY --chmod — Docker Documentation](https://docker.com)
*   [uutils/coreutils — проблема сумісності з AppArmor / Ubuntu](https://github.com)
*   [Ubuntu: env-shebang scripts compatibility issue](https://ubuntu.com)
