/**
 * @file PresMeasUnitPOC.ino
 * @brief Proof of Concept for pressure measurement using BMP280 (I2C) 
 * and an Analog Pressure Transducer (MPX2202) via ADS1015 ADC.
 * * Hardware Setup:
 * - BMP280: I2C Address 0x76 (Pressure/Temperature)
 * - ADS1015: I2C Address 0x48 (Differential reading on Pins 0 and 1)
 */

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>

// --- Hardware Objects ---
Adafruit_BMP280 bmp;    // Barometric pressure sensor object
Adafruit_ADS1015 ads;   // Analog-to-Digital converter object

// --- Constants & Calibration ---
const float MAX_HPA = 2000.0;               // Max pressure range of the analog sensor (hPa)
const float SENS_10V = 0.040;               // Sensitivity coefficient at 10V reference
const float ACTUAL_VCC = 4.995;             // Measured VCC supply to the sensors
const int OVERSAMPLE_COUNT = 64;            // Number of samples for ADC averaging
const unsigned long MEASURE_INTERVAL = 100; // Time between measurements (ms)
const float ADS_VOLTS_PER_BIT = 0.000125f;  // Voltage conversion factor for GAIN_SIXTEEN

// --- Global Variables ---
float dynamic_factor;           // Calculated scaling factor for voltage-to-pressure conversion
float pressure_offset = 0;      // Offset to align Analog sensor with BMP280 reference
unsigned long lastMeasureTime = 0;
bool isMeasuring = true; 
String inputString = "";

/**
 * @brief Hardware initialization and initial calibration.
 */
void setup() {
  Serial.begin(9600);

  // Initialize I2C with High Speed Clock (400kHz)
  Wire.begin();
  Wire.setClock(400000);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Error: BMP280 not found"));
    while(1);
  }
  
  if (!ads.begin(0x48)) {
    Serial.println(F("Error: ADS1015 not found"));
    while(1); 
  }

  // Set Gain to +/- 0.256V for high resolution on small differential signals
  ads.setGain(GAIN_SIXTEEN); 

  // Calculate scaling factor based on actual supply voltage
  float current_sensitivity = SENS_10V * (ACTUAL_VCC / 10.0);
  dynamic_factor = MAX_HPA / current_sensitivity;

  Serial.println(F("=== System Ready ==="));
  Serial.println(F("Commands: ST (Start), SP (Stop), CAL (Calibrate)"));
  
  performCalibration();
}

/**
 * @brief Main execution loop: handles serial commands and timing-based sampling.
 */
void loop() {
  handleUART();

  if (isMeasuring && (millis() - lastMeasureTime >= MEASURE_INTERVAL)) {
    lastMeasureTime = millis();
    takeMeasurement();
  }
}

/**
 * @brief Non-blocking check for incoming Serial data.
 */
void handleUART() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      executeCommand(inputString);
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}

/**
 * @brief Executes logic based on received UART string commands.
 * @param cmd The trimmed input string.
 */
void executeCommand(String cmd) {
  cmd.trim();

  if (cmd == "ST") {
    isMeasuring = true;
    Serial.println(F("Measurement Started"));
  }
  else if (cmd == "SP") {
    isMeasuring = false;
    Serial.println(F("Measurement Stopped"));
  }
  else if (cmd == "CAL") {
    performCalibration();
  }
}

/**
 * @brief Calibrates the analog sensor by comparing it against the BMP280 digital reference.
 * Takes 100 samples to establish a baseline pressure offset.
 */
void performCalibration() {
  Serial.println(F("Calibrating..."));
  float bmp_acc = 0;
  float ads_volts_acc = 0;
  const int samples = 100;

  for(int i = 0; i < samples; i++) {
    bmp_acc += (bmp.readPressure() / 100.0F); // Convert Pa to hPa
    ads_volts_acc += ads.computeVolts(ads.readADC_Differential_0_1());
    delay(5);
  }

  float avg_bmp = bmp_acc / (float)samples;
  float avg_ads_volts = ads_volts_acc / (float)samples;
  
  // Calculate difference between "True" BMP pressure and raw Analog pressure
  pressure_offset = avg_bmp - (avg_ads_volts * dynamic_factor);

  Serial.print(F("New Offset: ")); 
  Serial.println(pressure_offset);
}

/**
 * @brief Performs oversampled reading of ADC and outputs CSV formatted data.
 * Format: [BMP280_hPa],[Analog_Sensor_hPa]
 */
void takeMeasurement() {
  float bmp_val = bmp.readPressure() / 100.0F;
  float raw_accumulator = 0;

  // Oversampling loop for noise reduction
  for(int i = 0; i < OVERSAMPLE_COUNT; i++) {
    raw_accumulator += (float)ads.readADC_Differential_0_1();
  }
  float raw_avg = raw_accumulator / (float)OVERSAMPLE_COUNT;

  // Convert raw ADC bits to Pressure (hPa)
  float v_avg = raw_avg * ADS_VOLTS_PER_BIT; 
  float ads_val = (v_avg * dynamic_factor) + pressure_offset;

  // Output
  Serial.print(bmp_val, 2); 
  Serial.print(",");
  Serial.println(ads_val, 2);
}
