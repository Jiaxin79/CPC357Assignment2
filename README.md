# Smart Home - Smart Air Conditioning and Fan Control System

## Overview

This project implements a **Smart Air Conditioning and Fan Control System**, designed to optimize energy usage and maintain a comfortable indoor environment by intelligently switching between fan and air conditioner modes based on temperature and humidity levels.

## Features

- **Temperature-Based Control:**  
  - If the temperature is between **26°C and 30°C**, the system will **turn on the fan**.
  - If the temperature exceeds **30°C** and humidity is greater than **40%**, the system will:
    - **Turn off the fan.**
    - **Activate the air conditioner in Dry mode (LED Green),** which reduces humidity efficiently with lower energy consumption.
  - If the temperature exceeds **30°C** and humidity is less than **40%**, the system will:
    - **Switch the air conditioner to Cool mode (LED Yellow)** to maintain humidity levels.

