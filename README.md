# **PerfectSip Temperature Control Mug**
Welcome to the PerfectSip Temperature Control Mug GitHub repository! This project aims to develop a portable mug that keeps your beverage at the perfect temperature, controllable via a mobile app on your smartphone.

## **Project Overview**
The PerfectSip Temperature Control Mug features:

**Temperature Control:** Set your desired beverage temperature through the mobile app.
**LED Temperature Display:** LEDs on the mug indicate the current temperature of your drink.
**Audible Notification:** A buzzer notifies you when your beverage reaches the desired temperature.
**Mobile App:** Control and monitor your mug's temperature via a mobile app built on Xcode using Swift.
**Bluetooth Communication:** Utilizes the Core Bluetooth module to communicate with the HM-19 Bluetooth module.
**Power Source:** Powered by 3 3.7V 18650 lithium-ion batteries, charged with a balance charger via a standard 110/220V wall outlet.

### Stretch Goals
In addition to the core features, our stretch goals include:

**Audio Amplifier:** Integrate a high-fidelity speaker for complex sounds.

**Battery Monitoring:** Track battery voltage using ADC and monitor heater usage time.

## Project Specific Design Requirements (PSDRs)
To ensure the functionality of the PerfectSip mug, we have defined the following PSDRs:

**Hardware Communication:** Communicate with the HM-19 Bluetooth module via UART for temperature data exchange.

**LED Control:** Control 16 LEDs using shift registers for serial-to-parallel data conversion.

**Power Boosting:** Utilize a buck converter to boost power from 11.1V to 24V for heating.

**Temperature Sensing:** Interface with the microcontroller's ADC to read temperature values from a thermal resistor voltage divider.

**Software Control:** Develop a mobile app on Xcode (Swift) to control and monitor the mug's temperature via Bluetooth.



## Thank you for your interest in our project! Happy sipping! 🍵✨





