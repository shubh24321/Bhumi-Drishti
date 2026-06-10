# Bhumi Drishti 🛰️
### 1U CubeSat High-Altitude Balloon Mission

[![Mission Status](https://img.shields.io/badge/status-Phase%202%20Build-brightgreen)]()
[![Platform](https://img.shields.io/badge/platform-Arduino-blue)]()
[![Altitude Target](https://img.shields.io/badge/altitude-20--40%20km-orange)]()
[![License](https://img.shields.io/badge/license-MIT-lightgrey)]()

A student-built 1U CubeSat (100×100×113.5 mm) flown on a weather balloon
to the stratosphere. Sensors measure pressure, temperature, magnetic field,
and GPS coordinates throughout ascent and descent.

## Mission Overview

- **Platform:** 1U CubeSat form factor (100 × 100 × 113.5 mm)
- **Launch vehicle:** 600g weather balloon
- **Target altitude:** 20,000 – 40,000 m (stratosphere)
- **Flight computer:** Arduino Nano
- **Sensors:** BMP280 (pressure + temp) · HMC5883L (magnetometer) · Neo-6M (GPS)
- **Data storage:** SD card, CSV format
- **Recovery:** Parachute + GSM backup tracker

## Repository Structure
```
Bhumi-Drishti/
├── firmware/          # Arduino .ino source files
├── hardware/          # Wiring diagrams, PCB files
├── data/              # Sample CSV logs from tests
├── docs/              # Mission plan, test reports
└── analysis/          # Post-flight data scripts
```

## Current Status
- [x] Phase 1 — Team formed, workspace created
- [ ] Phase 2 — Sensor integration & demo build
- [ ] Phase 3 — Environmental testing
- [ ] Phase 4 — Launch preparation & DGCA approval
- [ ] Phase 5 — Launch & recovery

## Team
Built by a 4-person student team based in India.

## License
MIT — open science, open data.

## System Architecture
![System Architecture](docs/your-image-filename.png)
