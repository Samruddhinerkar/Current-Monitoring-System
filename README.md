# Current-Monitoring-System

## Overview

This project is an IoT-based Current Monitoring System that measures AC current in real time. The current data is sent to ThingSpeak, where it is stored and displayed as graphs. The system helps monitor power usage and detect overload conditions.

## Features

* Measures AC current in real time
* Uploads data to ThingSpeak
* Displays current values using graphs
* Helps monitor power consumption
* Detects high current levels

## Components Used

* ESP32
* SCT-013-000 Current Sensor
* ADS1115 ADC
* Jumper Wires
* Breadboard
* Wi-Fi Connection

## Software Used

* Arduino IDE
* ThingSpeak
* ESP32 Board Package
* ADS1115 Library

## Working

1. The SCT-013-000 sensor measures the AC current.
2. The ADS1115 converts the analog signal into digital data.
3. The ESP32 reads the current values.
4. The ESP32 sends the data to ThingSpeak using Wi-Fi.
5. ThingSpeak stores the data and displays it as graphs.

## Applications

* Home energy monitoring
* Motor current monitoring
* Small industrial monitoring
* Power usage analysis

## Future Improvements

* Add SMS or email alerts for overload conditions
* Store data locally using an SD card
* Create a mobile application for monitoring
* Add voltage and power measurement

