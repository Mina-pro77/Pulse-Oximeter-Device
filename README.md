# 🫀 Arduino Pulse Oximeter

A simple **Arduino-based Pulse Oximeter** prototype designed to estimate **Heart Rate (BPM)** and **Blood Oxygen Saturation (SpO₂)** from an analog photoplethysmography (PPG) signal.

The project demonstrates the basic principles of analog signal conditioning, peak detection, heart-rate calculation, and SpO₂ estimation using an Arduino and a standard 16×2 LCD.

> ⚠️ **Disclaimer:** This project is an educational prototype and is **not a medical device**. The displayed SpO₂ and BPM values should not be used for medical diagnosis or clinical decisions.

---

## 📌 Project Overview

The system processes two analog signals:

* **DC component:** Represents the average optical signal received from the sensor.
* **AC component:** Represents the pulsatile component caused by blood flow.

The analog signals are processed externally using signal-conditioning circuits before being connected to the Arduino analog inputs.

The Arduino then:

1. Reads the DC signal.
2. Reads the amplified AC signal.
3. Detects the pulse waveform.
4. Detects signal peaks.
5. Calculates heart rate in BPM.
6. Estimates SpO₂ using the AC/DC ratio.
7. Displays the results on a 16×2 LCD.

---

## ✨ Features

* ❤️ Heart Rate measurement in **BPM**
* 🫁 Approximate **SpO₂ estimation**
* 📈 Analog PPG signal processing
* 🔎 Peak detection
* ⏱️ Heartbeat interval measurement using `millis()`
* 📺 16×2 LCD display
* ⚙️ Adjustable DC and AC gain compensation
* 🔧 Adjustable diode-drop compensation
* 🧪 Designed primarily for educational and experimental purposes

---

## 🧩 System Architecture

```text
        PPG / Optical Sensor
                │
                ▼
        Analog Signal
                │
        ┌───────┴────────┐
        │                │
        ▼                ▼
   DC Extraction     AC Extraction
        │                │
        │          HPF / Notch Filter
        │                │
        │          Amplification
        │                │
        │             Diode
        │                │
        └───────┬────────┘
                │
                ▼
             Arduino
                │
        ┌───────┴────────┐
        │                │
        ▼                ▼
   BPM Calculation   SpO₂ Estimation
        │                │
        └───────┬────────┘
                ▼
             16×2 LCD
```

---

## 🔌 Hardware

### Main Components

| Component                     | Description                           |
| ----------------------------- | ------------------------------------- |
| Arduino                       | Main microcontroller                  |
| 16×2 LCD                      | User interface                        |
| PPG Sensor / Analog Front-End | Optical pulse signal source           |
| HPF                           | High-pass filtering for AC extraction |
| Notch Filter                  | Power-line/interference rejection     |
| Amplifier                     | AC signal amplification               |
| Diode                         | Signal rectification                  |
| Resistors & Capacitors        | Analog signal conditioning            |
| 5V Supply                     | Arduino and analog circuit supply     |

---

## 🔗 Arduino Pin Configuration

### LCD

The project uses the standard Arduino `LiquidCrystal` library.

| LCD Pin | Arduino Pin |
| ------- | ----------: |
| RS      |         D12 |
| E       |         D11 |
| D4      |          D5 |
| D5      |          D4 |
| D6      |          D3 |
| D7      |          D2 |

The LCD is initialized using:

```cpp
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
```

### Analog Inputs

| Signal    | Arduino Pin |
| --------- | ----------- |
| DC Signal | A0          |
| AC Signal | A1          |

---

## ⚙️ Signal Conditioning

The analog front-end is assumed to provide two different signals.

### DC Channel

The DC channel is connected directly to:

```text
A0
```

The software compensates for the analog-stage gain:

```cpp
const float Gain_DC = 6.0;
```

The actual DC voltage is calculated using:

```cpp
trueDC = (rawDC * (5.0 / 1023.0)) / Gain_DC;
```

---

### AC Channel

The AC signal passes through the following stages before reaching the Arduino:

```text
PPG Signal
   ↓
High-Pass Filter
   ↓
Notch Filter
   ↓
Amplifier
   ↓
Diode / Rectifier
   ↓
Arduino A1
```

The AC gain is defined as:

```cpp
const float Gain_AC = 40.0;
```

The diode forward voltage is compensated using:

```cpp
const float DiodeDrop = 0.44;
```

This value should be experimentally adjusted according to the diode actually used.

Typical examples:

```text
Silicon diode   ≈ 0.6–0.7 V
Schottky diode  ≈ 0.2–0.4 V
```

The actual value depends on the diode type and operating current.

---

# 🧠 Software Algorithm

## 1. DC Measurement

The Arduino reads the DC channel using:

```cpp
analogRead(A0);
```

The ADC value is converted to voltage using the Arduino's 10-bit ADC:

```text
ADC Range = 0–1023
ADC Reference = 5 V
```

Therefore:

```text
VADC = ADC × 5 / 1023
```

After compensating for the amplifier gain:

```text
VDC = VADC / Gain_DC
```

---

## 2. AC Peak Detection

The AC signal is continuously sampled from:

```text
A1
```

The software tracks the maximum value of the positive portion of the waveform.

The basic principle is:

```text
        Peak
         ▲
         │
    /\   │
   /  \  │
__/    \_│____
```

The variable:

```cpp
currentPeakAC
```

stores the currently detected peak.

When the signal starts decreasing, the detected peak is transferred to:

```cpp
detectedPeakAC
```

---

## 3. Heartbeat Detection

The program detects a new pulse when the signal crosses the defined threshold.

The time between consecutive detected beats is calculated using:

```cpp
millis()
```

The heartbeat interval is:

```text
Δt = Current Time - Previous Beat Time
```

Then BPM is calculated using:

```text
BPM = 60000 / Δt
```

For example:

```text
Δt = 1000 ms

BPM = 60000 / 1000
    = 60 BPM
```

A minimum interval of approximately **300 ms** is used to prevent unrealistically fast detections.

---

# 🫁 SpO₂ Estimation

The project uses the relationship between the AC and DC components of the optical signal.

The basic ratio is:

```text
Ratio = AC / DC
```

In the current experimental implementation:

```cpp
ratio = trueAC_Peak / trueDC;
```

The displayed SpO₂ value is then estimated using:

```cpp
spo2 = 99 - 150 × ratio;
```

with the result constrained between:

```text
90% and 100%
```

### Important

The coefficient:

```cpp
150
```

is an **experimental calibration factor**, not a clinically validated SpO₂ conversion equation.

For a real pulse oximeter, SpO₂ estimation normally requires:

* Red LED measurement
* Infrared LED measurement
* Synchronized sampling
* AC/DC calculation for both wavelengths
* Ratio-of-ratios calculation
* Empirical calibration against reference measurements

Therefore, the current implementation should be considered an **educational approximation**.

---

# 📺 LCD Output

The system displays:

```text
BPM: 75
SpO2: 98%
```

If a valid heart rate is not detected, the display shows:

```text
BPM: --
```

The accepted display range in the current code is approximately:

```text
40 < BPM < 180
```

---

# 📁 Project Structure

A recommended GitHub project structure is:

```text
Arduino-Pulse-Oximeter/
│
├── src/
│   └── main.cpp
│
├── include/
│
├── lib/
│
├── docs/
│   ├── block-diagram.png
│   ├── schematic.png
│   └── prototype.jpg
│
├── README.md
├── LICENSE
└── platformio.ini
```

If the project is developed using Arduino IDE instead of PlatformIO, the structure can simply be:

```text
Arduino-Pulse-Oximeter/
│
├── Pulse_Oximeter.ino
├── README.md
├── LICENSE
└── docs/
    ├── schematic.png
    └── prototype.jpg
```

---

# 🛠️ Software Requirements

* Arduino IDE
* Arduino AVR Core
* `LiquidCrystal` library

The project uses:

```cpp
#include <Arduino.h>
#include <LiquidCrystal.h>
```

---

# 🚀 Getting Started

## 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/Arduino-Pulse-Oximeter.git
```

## 2. Open the Project

Open the project using:

* Arduino IDE

or, if using PlatformIO:

* Visual Studio Code + PlatformIO

## 3. Connect the Hardware

Connect the LCD and analog signals according to the pin configuration described above.

## 4. Upload the Firmware

Compile and upload the firmware to the Arduino board.

## 5. Monitor the Output

After startup, the LCD displays:

```text
Pulse Oximeter
System Ready
```

After approximately two seconds, the measurement screen appears.

---

# 🔧 Calibration

The following parameters can be adjusted according to the actual analog circuit:

```cpp
const float Gain_DC = 6.0;
const float Gain_AC = 40.0;
const float DiodeDrop = 0.44;
```

### DC Gain

Change:

```cpp
Gain_DC
```

to match the actual gain of the DC signal-conditioning circuit.

### AC Gain

Change:

```cpp
Gain_AC
```

to match the actual AC amplifier gain.

### Diode Drop

Change:

```cpp
DiodeDrop
```

according to the measured forward voltage of the rectifier diode.

### SpO₂ Calibration

The experimental coefficient:

```cpp
150
```

can be modified during testing:

```cpp
spo2 = 99 - (int)(150 * ratio);
```

However, this should **not** be interpreted as clinical calibration.

---

# 📊 Example Output

```text
----------------
| BPM: 72      |
| SpO2: 98%    |
----------------
```

When no valid pulse is detected:

```text
----------------
| BPM: --      |
| SpO2: 99%    |
----------------
```

---

# ⚠️ Limitations

This prototype has several limitations:

* The SpO₂ calculation is experimental.
* No red/IR ratio-of-ratios algorithm is currently implemented.
* ADC reference accuracy affects voltage calculations.
* Analog noise can affect peak detection.
* Motion artifacts can produce incorrect BPM readings.
* Diode forward voltage varies with current and temperature.
* The current threshold-based pulse detection is relatively simple.
* The algorithm has not been clinically validated.

Therefore, this project should be used for **learning, experimentation, and embedded-systems development only**.

---

# 🔮 Future Improvements

Possible improvements include:

* [ ] Implement dual-wavelength Red/IR SpO₂ measurement
* [ ] Implement ratio-of-ratios algorithm
* [ ] Add digital low-pass filtering
* [ ] Improve heartbeat peak detection
* [ ] Add adaptive thresholding
* [ ] Implement moving-average filtering
* [ ] Add signal-quality detection
* [ ] Reduce motion artifacts
* [ ] Replace blocking `delay()` calls with a fully non-blocking architecture
* [ ] Improve LCD user interface
* [ ] Add serial waveform monitoring
* [ ] Add real-time PPG graphing
* [ ] Add data logging
* [ ] Add Bluetooth/Wi-Fi connectivity
* [ ] Perform calibration against a reference pulse oximeter
* [ ] Design a dedicated PCB for the analog front-end
* [ ] Improve power management and battery operation

---

# 📚 Educational Concepts

This project demonstrates several important concepts in electronics and embedded systems:

### Analog Electronics

* Photoplethysmography (PPG)
* High-pass filtering
* Notch filtering
* Signal amplification
* Rectification
* DC/AC signal separation

### Embedded Systems

* ADC measurement
* GPIO interfacing
* LCD control
* Real-time signal processing
* Timing using `millis()`
* Peak detection

### Biomedical Electronics

* Optical pulse sensing
* Heart-rate estimation
* AC/DC analysis
* SpO₂ estimation principles

---

# 🧪 Testing Recommendations

For experimental testing:

1. Allow the analog circuit to stabilize.
2. Place the finger consistently on the optical sensor.
3. Avoid excessive movement.
4. Keep the sensor pressure relatively constant.
5. Observe the raw AC waveform if possible.
6. Adjust the analog gain to avoid ADC saturation.
7. Compare BPM with a reference device.
8. Treat SpO₂ values as experimental until properly calibrated.

---

# 📜 License

This project is intended for educational and experimental purposes.

You may modify and use the source code according to the terms of the selected license.

A suitable choice for this project is the **MIT License**.

---

# 👨‍💻 Author

**Mina Hany Wadie Aziz**

Electronics Engineering Student
Embedded Systems & PCB Design

Interested in:

* Embedded Systems
* PCB Design
* Analog Electronics
* IoT
* Biomedical Electronics
* Industrial Automation

---

# ⭐ Support the Project

If you find this project useful:

⭐ Give the repository a star on GitHub.

🍴 Fork the project and experiment with the design.

🐛 Open an issue if you find a problem.

💡 Contributions and improvements are welcome.

---

## 📌 Project Status

**Development Status:** 🟡 Experimental / Educational Prototype

The current version successfully demonstrates the basic concept of:

```text
Analog PPG Signal
        ↓
Signal Conditioning
        ↓
Arduino ADC
        ↓
Peak Detection
        ↓
BPM Calculation
        ↓
AC/DC Analysis
        ↓
SpO₂ Estimation
        ↓
LCD Display
```

> **Note:** This repository represents an educational prototype and is not intended for medical or diagnostic applications.
