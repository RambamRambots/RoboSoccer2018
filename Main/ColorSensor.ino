boolean initColorSensor() {
  // color sensor
  if (tcs.begin()) {
    Serial.println("Found sensor");
    return true;
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    return false;
  }
}

