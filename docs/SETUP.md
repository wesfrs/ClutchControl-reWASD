# ClutchControl-reWASD Setup Guide

## Complete Setup for reWASD Approach

### 1. Install reWASD
1. Download from: https://rewasd.com/download
2. Install (requires admin rights)
3. Free trial: 7 days, then purchase license (~$7-20)

### 2. Flash Lilygo Firmware
1. Open Arduino IDE
2. Open `firmware/lilygo_gyro_ble.ino`
3. Board: **ESP32S3 Dev Module**
4. Settings:
   - USB CDC On Boot: **Enabled**
   - Flash Size: **16MB (128Mb)**
   - Partition Scheme: **16M Flash (3M APP/9.9MB FATFS)**
   - PSRAM: **OPI PSRAM**
5. Upload → Open Serial Monitor (115200) → Wait for "Calibration complete!"

### 3. Pair Controllers

#### Lilygo T-Display S3 (BLE)
1. Windows Settings → Bluetooth & devices → Add device → Bluetooth
2. Select **"Lilygo Gyro"** → Pair (no passkey)
3. Verify in `joy.cpl` → Shows "Lilygo Gyro"

#### MOCUTE 061F
1. Hold **A + Power** for 3 seconds (pairing mode)
2. Windows Bluetooth → Add device → **MOCUTE-061F**
3. Passkey: **0000**
3. Verify in `joy.cpl` → Shows **"Xbox Series X Controller"**

### 4. reWASD Configuration

#### Step 1: Create Profile
1. Open reWASD
2. File → New Profile
3. Name: "ClutchControl Gyro"

#### Step 2: Add Devices
1. Click **"+"** → Add Device
2. Select **"MOCUTE-061F"** (Xbox Series X) → Set as **Slot 1**
3. Click **"+"** → Add Device
4. Select **"Lilygo Gyro"** (HID gamepad) → Set as **Slot 2**

#### Step 3: Map Lilygo Gyro → Right Stick
1. Select **Slot 2 (Lilygo Gyro)**
2. **Gyro X** → Right Stick X
   - Right-click Gyro X axis → Map to → Right Stick X
   - Sensitivity: **1.0x** (adjust to taste)
   - Deadzone: **5%**
2. **Gyro Y** → Right Stick Y
   - Map to → Right Stick Y
   - Sensitivity: **1.0x**
   - Deadzone: **5%**
3. **Gyro Z** → (Optional) Map to Right Stick Z or leave unmapped

#### Step 4: Configure MOCUTE (Slot 1)
1. Select **Slot 1 (MOCUTE)**
2. **Left Stick** → Left Stick (Movement)
3. **Right Stick** → **Disable** (or map to keyboard if needed)
   - We want Lilygo to control aiming
4. **Buttons** → All standard (A/B/X/Y, LB/RB, Start/Back, etc.)
5. **Triggers** → L2/R2 (L2 = Gyro activation if using toggle)

#### Step 5: Gyro Activation (Optional - L2 Toggle)
If you want L2 to toggle gyro:
1. Slot 1 (MOCUTE) → L2 (Left Trigger)
2. Map to → **Toggle** → Slot 2 (Lilygo) **Enable/Disable**
3. Or use reWASD's "Shift Layer" feature for hold-to-activate

#### Step 6: Apply
1. Click **Apply** (top toolbar)
2. Profile saved automatically

### 5. Test in Game
1. Launch game (RE Remake, CoD, Apex, etc.)
2. Game should detect **"Xbox 360 Controller"** (reWASD virtual)
3. Hold L2 on MOCUTE → Tilt Lilygo → Aim moves!

## Profile Import (Optional)
Import `config/rewased_profile.rwprofile` in reWASD:
1. File → Import Profile
2. Select the .rwprofile file
3. Apply

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Lilygo not detected in reWASD | Re-pair Bluetooth, ensure BLE HID mode |
| Gyro too sensitive | Lower sensitivity in reWASD axis settings |
| Gyro inverted | Check "Invert" checkbox on axis mapping |
| L2 not toggling | Check reWASD Shift Layer settings |
| Game doesn't see controller | Ensure reWASD "Apply" is clicked, restart game |

## reWASD Profile Export
To share your config:
1. reWASD → File → Export Profile
2. Save as `.rwprofile`
3. Copy to `config/rewased_profile.rwprofile`

## Resources
- reWASD Docs: https://help.rewasd.com/
- Community Profiles: https://discord.gg/rewasd