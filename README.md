# Pressure Measurement Unit - Chemsilium

A differential pressure acquisition system developed for the **Chemsilium Science Club**. This project integrates a **BMP280** (for ambient barometric pressure as calibration point) and dual *MPXM2202GS/AS* setup for differential pressure measurement.

## Repository Structure

The project is organized into four main branches to ensure maintainability:

* **`/hardware`**: Contains **KiCad** PCB and electronic design files.
  <!-- * `sch/`: Schematic diagrams. -->
  <!-- * `pcb/`: Board layout and routing. -->
  <!-- * `fabrication/`: Production-ready Gerbers, BOM (Bill of Materials), and CPL files. -->
* **`/firmware`**: Source code for the **Arduino/C++** embedded system.
  * Handles I2C communication with sensors.
  * Manages data packet formatting for serial transmission (using UART).
* **`/software`**: Desktop application scripts for data visualization and logging.
  * Real-time graphing of pressure trends.
  * CSV logging for post-experimental analysis.
* **`/docs`**: Technical documentation, including user manual with setup, maintenance and use explanations.

---
