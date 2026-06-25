# IIoT Flood Monitoring System

A simulated IoT-based flood monitoring system for a dam built using ESP32, 
ThingSpeak, and Discord Webhooks.

## Demo Video
[Click here to watch the demo](YOUR_YOUTUBE_LINK_HERE)](https://youtu.be/CtEWtpGdAwQ)

## Wokwi Simulation
[[Click here to open the circuit simulation](YOUR_WOKWI_PROJECT_LINK_HERE)](https://wokwi.com/projects/467595113038959617)

## ThingSpeak Dashboard
[[Click here to view live data](YOUR_THINGSPEAK_CHANNEL_LINK_HERE)](https://thingspeak.mathworks.com/channels/3413216)

## How It Works
The HC-SR04 ultrasonic sensor measures the distance to the water surface.
The ESP32 calculates the water level and checks it against two thresholds:

- Below 120 cm — SAFE (green LED)
- 120 cm and above — WARNING (yellow LED)  
- 160 cm and above — DANGER (red LED blinking + buzzer + Discord alert)

When the water level drops back to normal after a danger event, a second 
Discord notification is sent to inform authorities that the danger has passed.

## Cloud Platforms
- **ThingSpeak** — logs water level and status every 16 seconds as live charts
- **Discord Webhooks** — sends real-time alerts to authorities when danger 
threshold is crossed and all-clear when water returns to normal

## Components
### Hardware (simulated in Wokwi)
- ESP32 DevKit V1
- HC-SR04 Ultrasonic Sensor
- Green, Yellow, Red LEDs
- 330 Ohm Resistors
- Buzzer

### Software
- Arduino C++
- Wokwi Online Simulator

## Pin Connections
| Component | Pin | ESP32 |
|---|---|---|
| HC-SR04 | TRIG | D5 |
| HC-SR04 | ECHO | D18 |
| Green LED | anode | D25 |
| Yellow LED | anode | D26 |
| Red LED | anode | D27 |
| Buzzer | + | D14 |

## Alert System
| Status | LED | Buzzer | Discord |
|---|---|---|---|
| Safe | Green ON | OFF | No alert |
| Warning | Yellow ON | OFF | No alert |
| Danger | Red BLINKING | ON | Flood alert sent |
| Back to normal | Green ON | OFF | All clear sent |

## Files
| File | Description |
|---|---|
| sketch.ino | ESP32 firmware |
| diagram.json | Wokwi circuit diagram |
| flowchart.png | Code logic flowchart |
| Flood_Monitoring_Lab_Report.pdf | Full project report |
