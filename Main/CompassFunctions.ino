#define COMPASS_DELAY 100

boolean initCompass() {
  if(!bno.begin()) {
    Serial.println("No compass found.");
    while(1);
  } else {
    Serial.println("Compass found. Calibrating...");
  }
  delay(500);
  bno.setExtCrystalUse(true);

  uint8_t system, gyro, accel, mag = 0;
  char status[5];
  while(mag != 3) {
    bno.getCalibration(&system, &gyro, &accel, &mag);
    sprintf(status,"CAL%i",mag);
    display.clearDisplay();
    line1(F("Calibrate Compass"));
    line2("Status: " + String(status));
    delay(100);
  }
  Serial.println("The compass is set up.");
  delay(100);
  return true;
}

void displaySensorDetails(void) {
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print("Sensor:       "); Serial.println(sensor.name);
  Serial.print("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
  Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
  Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void displaySensorStatus(void) {
  /* Get the system status values (mostly for debugging purposes) */
  uint8_t system_status, self_test_results, system_error;
  system_status = self_test_results = system_error = 0;
  bno.getSystemStatus(&system_status, &self_test_results, &system_error);

  /* Display the results in the Serial Monitor */
  Serial.println("");
  Serial.print("System Status: 0x");
  Serial.println(system_status, HEX);
  Serial.print("Self Test:     0x");
  Serial.println(self_test_results, HEX);
  Serial.print("System Error:  0x");
  Serial.println(system_error, HEX);
  Serial.println("");
  delay(500);
}

void displayCalStatus(void)
{
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  char message[32];
  bno.getCalibration(&system, &gyro, &accel, &mag);
  Serial.print("Sys:"); Serial.print(system, DEC);

  sprintf(message, "%d", system);
  line2(message);
}

void displaySensorOffsets(const adafruit_bno055_offsets_t &calibData)
{
  Serial.print("Accelerometer: ");
  Serial.print(calibData.accel_offset_x); Serial.print(" ");
  Serial.print(calibData.accel_offset_y); Serial.print(" ");
  Serial.print(calibData.accel_offset_z); Serial.print(" ");

  Serial.print("\nGyro: ");
  Serial.print(calibData.gyro_offset_x); Serial.print(" ");
  Serial.print(calibData.gyro_offset_y); Serial.print(" ");
  Serial.print(calibData.gyro_offset_z); Serial.print(" ");

  Serial.print("\nMag: ");
  Serial.print(calibData.mag_offset_x); Serial.print(" ");
  Serial.print(calibData.mag_offset_y); Serial.print(" ");
  Serial.print(calibData.mag_offset_z); Serial.print(" ");

  Serial.print("\nAccel Radius: ");
  Serial.print(calibData.accel_radius);

  Serial.print("\nMag Radius: ");
  Serial.print(calibData.mag_radius);
}


void calibrateCompass(void) {
  return;
  Serial.println("Orientation Sensor Test"); Serial.println("");

  int eeAddress = 0;
  long bnoID;
  bool foundCalib = false;

  EEPROM.get(eeAddress, bnoID);

  adafruit_bno055_offsets_t calibrationData;
  sensor_t sensor;

  /*
     Look for the sensor's unique ID at the beginning oF EEPROM.
     This isn't foolproof, but it's better than nothing.
  */
  bno.getSensor(&sensor);
  if (bnoID != sensor.sensor_id)
  {
    Serial.println("\nNo Calibration Data for this sensor exists in EEPROM");
    delay(500);
  }
  else
  {
    Serial.println("\nFound Calibration for this sensor in EEPROM.");
    eeAddress += sizeof(long);
    EEPROM.get(eeAddress, calibrationData);

    // displaySensorOffsets(calibrationData);

    Serial.println("\n\nRestoring Calibration data to the BNO055...");
    bno.setSensorOffsets(calibrationData);

    Serial.println("\n\nCalibration data loaded into BNO055");
    foundCalib = true;
    line2(F("Loaded"));
  }
  delay(500);
  // displaySensorDetails();
  // displaySensorStatus();

  sensors_event_t event;
  bno.getEvent(&event);
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  char message[32];
  bno.getCalibration(&system, &gyro, &accel, &mag);
  if (foundCalib) {
    Serial.println("Move sensor slightly to calibrate magnetometers");
    line2(F("WIGGLE"));
    while (system < 3) {
      bno.getEvent(&event);
      delay(COMPASS_DELAY);
    }
  }
  else {
    Serial.println(F("Please Calibrate Sensor: "));
    while (system < 3) {
      bno.getEvent(&event);
      displayCalStatus();
      Serial.println("");
      delay(COMPASS_DELAY);
    }
  }

  Serial.println("\nFully calibrated!");
  Serial.println("--------------------------------");
  Serial.println("Calibration Results: ");
  adafruit_bno055_offsets_t newCalib;
  bno.getSensorOffsets(newCalib);
  displaySensorOffsets(newCalib);

  Serial.println("\n\nStoring calibration data to EEPROM...");
  eeAddress = 0;
  bno.getSensor(&sensor);
  bnoID = sensor.sensor_id;

  EEPROM.put(eeAddress, bnoID);

  eeAddress += sizeof(long);
  EEPROM.put(eeAddress, newCalib);
  Serial.println("Data stored to EEPROM.");

  Serial.println("\n--------------------------------\n");
  delay(500);
}

double readCompass() {
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  double heading = euler.x();
//  line3(String(heading));
  return heading;
}

double getCompOff() {
  double offset = readCompass() - front;
  if (offset > 180) {
    offset -= 360;
  } else if (offset < -180) {
    offset += 360;
  }
//  line2(String(offset));
  return offset;
}

