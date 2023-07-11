# DFROBOT LCD I2C Library

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## Description

The DFROBOT LCD I2C Library is a simple Arduino library that provides an interface to control LCD displays using the I2C communication protocol. This library is designed to work specifically with DFROBOT I2C LCD displays, making it easy to integrate and utilize the LCD functionality in your Arduino projects.

## Features

- Supports both 16x2 and 20x4 LCD display modules.
- Provides simple functions to initialize the display, set the cursor position, clear the display, and print text.
- Customizable backlight control for supported displays.
- Optimized I2C communication for improved performance.

## Installation

1. Clone or download the repository.
2. Move the `DFROBOTLCD_I2CLibrary` folder to your Arduino libraries directory.
   - On Windows: `Documents\Arduino\libraries`
   - On macOS: `Documents/Arduino/libraries`
   - On Linux: `~/Arduino/libraries`
3. Restart the Arduino IDE if it was already running.
4. You can now include the library in your Arduino sketches by selecting **Sketch > Include Library > DFROBOTLCD_I2CLibrary**.

## Usage

1. Connect your DFROBOT I2C LCD display to your Arduino board following the wiring instructions provided in the `examples` directory.
2. Open the Arduino IDE and navigate to **File > Examples > DFROBOTLCD_I2CLibrary** to access the example sketches.
3. Choose an example sketch and upload it to your Arduino board.
4. You should see the relevant information displayed on the LCD screen.

For detailed usage instructions and available functions, refer to the [library documentation](docs/README.md).

## Contributing

Contributions are welcome and greatly appreciated. To contribute to the DFROBOT LCD I2C Library, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your modifications and ensure they adhere to the existing code style.
4. Test your changes to ensure they work as expected.
5. Commit your changes and push them to your fork.
6. Submit a pull request, clearly describing the changes you made.

Please review the [contribution guidelines](CONTRIBUTING.md) for more information.

## License

This project is licensed under the [MIT License](LICENSE). You are free to modify and distribute the library. However, any contributions you make must be provided under the same MIT license.

## Contact

For any questions or suggestions, feel free to open an issue on the [GitHub repository](https://github.com/hot-tea-and-driver-updates/DFROBOTLCD_I2CLibrary/issues).
