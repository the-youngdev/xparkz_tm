# XParkz_TM 🤖

XParkz_TM is a modular, high-performance line follower robot firmware written in C++ for Arduino.  
It is designed with clean architecture, on-device tuning, and competition reliability in mind.

---

## 🚀 Features
- PID-based line following
- OLED menu-driven UI
- On-device tuning (no laptop required)
- EEPROM-backed parameter storage
- ADS1115-based sensor input
- Line inversion support
- Stop-box & checkpoint detection
- Modular and scalable codebase

---

## 🧠 Project Architecture
```
src/
├── main.cpp
├── config.h
├── motors.cpp / motors.h
├── sensors.cpp / sensors.h
├── ui.cpp / ui.h
```

---

## 🔌 Hardware
- Arduino Nano / Uno
- TB6612FNG Motor Driver
- ADS1115 ADC
- SH1106 OLED Display
- Line sensor array
- Push buttons, buzzer, LED

---

## 🛠️ Build & Upload

### PlatformIO (Recommended)
```
platformio run
platformio run --target upload
```

### Arduino IDE
Install required libraries:
- Adafruit ADS1X15
- U8g2

---

## 💾 EEPROM
PID values, speed, and calibration data are stored in EEPROM and restored on boot.

---

## 📈 Future Plans
- Bluetooth tuning
- Lap timing
- ESP32 support
- Advanced state-machine navigation

---

## 👤 Author
Soumyadip Das  
GitHub: https://github.com/the-youngdev

---

## 📜 License
MIT License
