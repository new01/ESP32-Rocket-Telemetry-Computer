#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD_MMC.h>
#include "sd_read_write.h"

#define SD_MMC_CMD 15 //Please do not modify it.
#define SD_MMC_CLK 14 //Please do not modify it. 
#define SD_MMC_D0  2  //Please do not modify it.

Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;

const char* baseFilename = "/data";
const char* fileExtension = ".csv";
String filename;

void setup() {
  Serial.begin(115200);
  //Wire.begin(21, 22); // Initialize I2C with SDA=21 and SCL=22

  // Initialize BME280
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Initialize IMU 6050
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1);
  }

  // MPU 6050 config
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  
  // Initialize SD card
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
      Serial.println("Card Mount Failed");
      return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return;
    }


  // Find the next available file number
  int fileNumber = 1;
  while (SD_MMC.exists(getFilename(fileNumber).c_str())) {
    fileNumber++;
  }

  // Generate the filename
  filename = getFilename(fileNumber);

  // Create or open the CSV file and write the header row
  writeFile(SD_MMC, filename.c_str(), "Time(s:ms),Altitude(m),AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ\n");
}

void loop() {
  // Read data from BME280
  float altitude = bme.readAltitude(1013.25); // Adjust sea level pressure as needed

  // Read data from MPU6050
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Get time elapsed since boot in seconds with 3 decimal points
  float elapsedTime = millis() / 1000.0;

  // Prepare the data string
  String dataString = String(elapsedTime, 3) + ",";
  dataString += String(altitude) + ",";
  dataString += String(accel.acceleration.x) + ",";
  dataString += String(accel.acceleration.y) + ",";
  dataString += String(accel.acceleration.z) + ",";
  dataString += String(gyro.gyro.x) + ",";
  dataString += String(gyro.gyro.y) + ",";
  dataString += String(gyro.gyro.z) + "\n";

  // Append the data to the file
  appendFile(SD_MMC, filename.c_str(), dataString.c_str());

  delay(100); // Delay between readings (100 ms = 0.1 seconds)
}

// Function to generate the filename based on the file number
String getFilename(int number) {
  return String(baseFilename) + String(number) + String(fileExtension);
}