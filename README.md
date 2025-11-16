# FlipperZero-SpyCamDetector
A professional-grade security tool for Flipper Zero that detects wireless spy cameras and surveillance devices using advanced RF signal analysis. This application transforms your Flipper Zero into a powerful anti-surveillance device capable of identifying hidden cameras in hotels, Airbnb rentals, changing rooms, and private spaces.

# Flipper Zero Spy Camera Detector v1.1.0

A professional spy camera detection tool for Flipper Zero that scans for wireless surveillance devices using RF signal analysis.

## Features

- **Multi-frequency Scanning**: Detects cameras on 2.4GHz, 5GHz, 433MHz, and other common frequencies
- **Threat Level Assessment**: Rates detected signals from 1-4 based on risk
- **Visual & Haptic Alerts**: LED and vibration notifications for threats
- **Statistics Tracking**: Records scan duration and signal counts
- **Professional UI**: Clean interface with progress indicators and signal strength display

## Supported Frequencies

- 2.4GHz band (2400-2483.5 MHz) - Most common wireless cameras
- 5GHz band (5150-5850 MHz) - High-end surveillance devices  
- 433MHz & 240MHz - Some specialty wireless cameras
- Custom frequency database with threat assessment

## Installation

1. Download the `spy_camera_detector.fap` file from Releases
2. Copy to `apps/Tools/` on your Flipper Zero SD card
3. Launch via Applications → Tools → Spy Camera Detector

## Usage

### Basic Operation:
- **Right Button**: Start/Stop scanning
- **Left Button**: Reset statistics  
- **OK Button**: Toggle scan (alternative)

### Scanning Process:
1. Press Right button to start scanning
2. Device will scan through threat frequencies
3. Visual indicators show scanning progress
4. Threats trigger vibration and LED alerts
5. Check displayed frequency and threat level

### Threat Levels:
- **Level 1**: Weak signal, common frequency
- **Level 2**: Moderate signal or suspicious frequency  
- **Level 3**: Strong signal on known camera frequency
- **Level 4**: Very strong signal on confirmed camera frequency

## Technical Details

- Uses SubGhz subsystem for RF analysis
- Signal strength threshold: -75 dBm
- Scan interval: 100ms
- Database of 12 known threat frequencies
- Real-time signal analysis and classification

## Version History

### v1.1.0
- Enhanced threat level system (1-4)
- Improved signal analysis algorithms  
- Better visual feedback with progress bars
- Added signal strength indicators
- Reduced false positives
- Professional UI redesign

### v1.0.0
- Initial release
- Basic frequency scanning
- Simple threat detection

## Legal Notice

This tool is intended for:
- Security professionals conducting authorized audits
- Individuals checking their own private property  
- Educational and research purposes

Always ensure you have proper authorization before scanning any location.

## Contributing

Contributions welcome! Please submit issues and pull requests on GitHub.

## License

MIT License - See LICENSE file for details.
