# Smart-Door-Lock-System

## Project Overview

The **Smart Door Lock System** is a versatile security solution designed to provide secure and convenient access control for various environments. It combines multiple authentication methods, including **RFID**, **fingerprint recognition**, **keypad entry**, and **Bluetooth command control**. This system is implemented on an **Arduino Mega** microcontroller, integrating hardware modules and custom logic to create a robust security solution.

## Features

-   **Multi-factor Authentication**: Allows access using RFID cards, fingerprint scanning, a keypad password, or Bluetooth commands.
-   **Automated Door Control**: Controls a servo motor for door locking/unlocking based on successful authentication.
-   **Bluetooth Command Support**: Allows remote access control and password management via Bluetooth.
-   **User Feedback**: LCD and buzzer for clear user interaction and notifications.

## Hardware Components

-   **Arduino Mega 2560**: Main microcontroller for processing.
-   **RFID Module**: Provides RFID-based authentication.
-   **Fingerprint Sensor**: Adds biometric verification as a secure layer.
-   **Keypad (4x4 Matrix)**: Allows manual password entry.
-   **HC-05 Bluetooth Module**: Enables remote access and configuration via Bluetooth.
-   **Ultrasonic Sensor**: Detects proximity for automatic fingerprint scanner activation.
-   **LCD Display (16x2 with I2C)**: Shows status and instructions.
-   **Servo Motor**: Controls door locking mechanism.
-   **Buzzer**: Alerts users about the current status (e.g., door locked/unlocked).

## Project Structure

```plaintext
.
├── README.md                   # Project overview and instructions
├── LICENSE                     # License for the project
├── .gitignore                  # Git ignore file
├── docs                        # Documentation files
│   ├── architecture-diagram.png
│   ├── flowcharts
│   ├── usage-guide.md
│   ├── hardware-setup.md
│   ├── report.tex              # LaTeX report file
│   ├── bibliography.bib        # Bibliography for the report
│   └── report.pdf              # Compiled report in PDF format
├── hardware                    # Hardware-related documentation
│   ├── schematics              # Schematics for circuit connections
│   └── bill-of-materials.md    # List of materials used
├── code                        # Source code
│   ├── src                     # Main source files
│   │   ├── main.ino            # Main program
│   │   ├── rfid_module.h       # RFID module functions
│   │   ├── fingerprint_module.h # Fingerprint module functions
│   │   └── bluetooth_module.h  # Bluetooth module functions
│   ├── lib                     # Custom libraries
│   └── tests                   # Test scripts
│       └── test_main.ino       # Test program for main features
├── assets                      # Multimedia assets (e.g., images, videos)
│   └── demo-video.mp4
└── experiments                 # Experimental or testing code
    ├── rfid_test.ino
    └── fingerprint_test.ino
```

## Getting Started

### Prerequisites

-   **Arduino IDE**: Ensure the latest version is installed.
-   **Libraries**:
    -   `Servo.h`
    -   `LiquidCrystal_I2C.h`
    -   `SPI.h`
    -   `MFRC522.h`
    -   `Adafruit_Fingerprint.h`
    -   `Keypad.h`
    -   `EEPROM.h`
    -   `SoftwareSerial.h`

These libraries can be installed via the Arduino Library Manager.

### Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/yourusername/Smart-Door-Lock-System.git
    cd Smart-Door-Lock-System
    ```

2. Open the `code/src/main.ino` file in the Arduino IDE.

3. Connect your Arduino Mega to your computer and upload the program.

### Hardware Setup

Refer to the [hardware-setup.md](docs/hardware-setup.md) document in the `docs` folder for detailed wiring instructions.

### Usage

1. **System Start**: Upon startup, the system displays "Welcome, Group 2" on the LCD and enters standby mode, awaiting user interaction.

2. **Authentication Methods**:

    - **RFID**:
        - Present a registered RFID card to the RFID reader.
        - The system will scan and validate the card against the pre-stored UID.
        - If the UID matches, the door will unlock.
    - **Fingerprint**:
        - Approach the ultrasonic sensor to activate fingerprint mode.
        - Place your registered finger on the fingerprint scanner when prompted on the LCD.
        - The system will verify the fingerprint; if successful, the door will unlock.
    - **Keypad**:
        - Enter the 4-digit password on the keypad (default password is "9999").
        - Press `#` to submit.
        - If the password is correct, the door will unlock.
    - **Bluetooth**:
        - **Unlock**: Send the command `password-OPEN` via Bluetooth (replace `password` with the current password) to unlock the door remotely.
        - **Change Password**: Send the command `oldPassword-CHANGE-newPassword` to update the password (e.g., `9999-CHANGE-1234` to change to a new password "1234").
        - Bluetooth will provide confirmation messages such as "Password Updated" or "Invalid Command" based on the input.

3. **Door Control**: Upon successful authentication through any method, the servo motor will unlock the door. The door remains unlocked for 5 seconds, after which it relocks automatically and the system returns to standby mode.

### Bluetooth Command Format

-   **Unlock**: `password-OPEN`
-   **Change Password**: `oldPassword-CHANGE-newPassword`

Example commands:

-   `1234-OPEN`
-   `1234-CHANGE-5678`

### Experimental and Test Scripts

The `experiments` folder contains scripts for testing individual components, such as `rfid_test.ino` and `fingerprint_test.ino`. These are useful for debugging and ensuring individual modules function correctly before full system integration.

## Results and Discussion

Refer to the `docs/report.pdf` for detailed results and analysis of the project implementation and testing.

## Acknowledgments

-   [MFRC522 RFID Library](https://github.com/miguelbalboa/rfid)
-   [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
