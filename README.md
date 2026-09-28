# ClutchControl-reWASD

**Lilygo T-Display S3 + MOCUTE 061F Gyro Aiming using reWASD**

This approach uses **reWASD** (paid software) to merge two physical controllers into one virtual controller. No custom PC app needed - reWASD handles everything at kernel level.

## Architecture

```
Lilygo T-Display S3 (BLE Gyro) ───┐
                                  ├──→ reWASD (kernel driver) ──→ Virtual Controller → Game
MOCUTE 061F (Bluetooth) ──────────┘
```

## Why reWASD?

| Feature | Support |
|---------|---------|
| Merge 2 controllers | ✅ Native |
| Gyro → Right Stick | ✅ Native |
| Kernel-level virtual controller | ✅ Included |
| All games (Steam, EA, Ubisoft, etc.) | ✅ Works everywhere |
| DualSense emulation | ✅ Optional profile |
| No coding required | ✅ GUI configuration |

## Setup

### 1. Hardware
- **Lilygo T-Display S3** with BMI270 gyro
- **MOCUTE 061F** gamepad

### 2. Firmware (Lilygo)
Flash `firmware/lilygo_gyro_ble.ino` to Lilygo T-Display S3 via Arduino IDE.

The firmware:
- Reads BMI270 gyro via I2C (GPIO 17=SCL, 18=SDA)
- Sends gyro data via BLE HID (BleGamepad)
- Display shows BLE connection status

### 3. Pair Controllers
1. **Lilygo**: Pair via Windows Bluetooth → "Lilygo Gyro"
2. **MOCUTE 061F**: Pair in `mocute-pc-xinput` mode (Xbox Series X)

### 4. reWASD Configuration
1. Install reWASD from https://rewasd.com/
2. Create new profile
3. **Add both controllers**:
   - Controller 1: MOCUTE 061F (Xbox Series X)
   - Controller 2: Lilygo Gyro (HID gamepad)
4. **Map Lilygo Gyro → Right Stick**:
   - Select Lilygo device
   - Map Gyro X → Right Stick X
   - Map Gyro Y → Right Stick Y
   - Set sensitivity/deadzone
5. **Set MOCUTE as primary**:
   - Left Stick → Left Stick
   - Right Stick → (disabled, handled by Lilygo)
   - All buttons → Same buttons
   - L2 → Gyro activation toggle (optional)
6. **Apply & Save**

## Folder Structure

```
ClutchControl-reWASD/
├── firmware/
│   └── lilygo_gyro_ble.ino      # BLE HID firmware for Lilygo
├── config/
│   └── rewased_profile.rwprofile # reWASD profile (import this)
├── docs/
│   ├── SETUP.md                  # Detailed setup guide
│   ├── REWASD_CONFIG.md          # reWASD mapping guide
│   └── TROUBLESHOOTING.md
└── README.md
```

## Advantages
- ✅ No custom PC app to maintain
- ✅ Kernel-level = works in ALL games (anti-cheat compatible)
- ✅ One-time GUI setup, no coding
- ✅ DualSense emulation built-in
- ✅ Active support & updates

## Disadvantages
- ❌ Paid software (~$7-20)
- ❌ Windows only
- ❌ Closed source

## License
MIT License