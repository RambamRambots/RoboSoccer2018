void selectSeeker(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(MULTIPLEXER);
  Wire.write(1 << i);
  Wire.endTransmission();
}

boolean initBallSensor() {
  uint8_t data;
  selectSeeker(0);
  if (!twi_writeTo(MULTIPLEXER, &data, 0, 1, 1)) {
    if (!twi_writeTo(SEEKER, &data, 0, 1, 1)) {
      irSensors[0] = true;
      irSensorCount++;
      InfraredSeeker::Initialize();
      Serial.println("SENSOR #1 INITIALIZED");
    }
    else {
      Serial.println("SENSOR #1 FAILURE");
    }
  }
  delay(100);

  selectSeeker(1);
  if (!twi_writeTo(MULTIPLEXER, &data, 0, 1, 1)) {
    if (!twi_writeTo(SEEKER, &data, 0, 1, 1)) {
      irSensors[1] = true;
      irSensorCount++;
      InfraredSeeker::Initialize();
      Serial.println("SENSOR #2 INITIALIZED");
    }
    else {
      Serial.println("SENSOR #2 FAILURE");
    }
  }
  delay(100);

  selectSeeker(2);
  if (!twi_writeTo(MULTIPLEXER, &data, 0, 1, 1)) {
    if (!twi_writeTo(SEEKER, &data, 0, 1, 1)) {
      irSensors[2] = true;
      irSensorCount++;
      InfraredSeeker::Initialize();
      Serial.println("SENSOR #3 INITIALIZED");
    }
    else {
      Serial.println("SENSOR #3 FAILURE");
    }
  }
  delay(100);

  selectSeeker(3);
  if (!twi_writeTo(MULTIPLEXER, &data, 0, 1, 1)) {
    if (!twi_writeTo(SEEKER, &data, 0, 1, 1)) {
      irSensors[3] = true;
      irSensorCount++;
      InfraredSeeker::Initialize();
      Serial.println("SENSOR #4 INITIALIZED");
    }
    else {
      Serial.println("SENSOR #4 FAILURE");
    }
  }

  if (irSensorCount == 4) {
    return true;
  } else {
    return false;
  }
}

int getReading(boolean retStrength) {
  int reading1, strength1;
  int reading2, strength2;
  int reading3, strength3;
  int reading4, strength4;
  InfraredResult ball1, ball2, ball3, ball4;

  if (irSensors[0]) {
    selectSeeker(0);
    ball1 = InfraredSeeker::ReadAC();
    reading1 = ball1.Direction - 5;
    strength1 = ball1.Strength / 8;
    if (reading1 == -5) {
      reading1 = 0;
      strength1 = 0;
    }
  }

  if (irSensors[1]) {
    selectSeeker(1);
    ball2 = InfraredSeeker::ReadAC();
    reading2 = ball2.Direction - 5;
    strength2 = ball2.Strength / 8;
    if (reading2 == -5) {
      reading2 = 0;
      strength2 = 0;
    }
  }

  if (irSensors[2]) {
    selectSeeker(2);
    ball3 = InfraredSeeker::ReadAC();
    reading3 = ball3.Direction - 5;
    strength3 = ball3.Strength / 8;
    if (reading2 == -5) {
      reading3 = 0;
      strength3 = 0;
    }
  }

  if (irSensors[3]) {
    selectSeeker(3);
    ball4 = InfraredSeeker::ReadAC();
    reading4 = ball4.Direction - 5;
    strength4 = ball4.Strength / 8;
    if (reading2 == -5) {
      reading4 = 0;
      strength4 = 0;
    }
  }

  char buffer[100];
  if (irSensorCount == 4) {
    int strengths[] = { strength1, strength2, strength3, strength4, -1 };
    int readings[] = { reading1, reading2, reading3, reading4, 6 };

    if (reading1 == 2 && reading2 == -2 && strength1 > 10 && strength2 > 10) {
      return retStrength ? max(strength1, strength2) : 0;
    } else if (reading2 == 2 && reading3 == -2 && strength2 > 10 && strength3 > 10) {
      return retStrength ? max(strength2, strength3) : 90;
    }  else if (reading3 == 2 && reading4 == -2 && strength3 > 10 && strength4 > 10) {
      return retStrength ? max(strength3, strength4) : 180;
    }  else if (reading4 == 2 && reading1 == -2 && strength4 > 10 && strength1 > 10) {
      return retStrength ? max(strength4, strength1) : -90;
    }
    
    int strongest = 0;
    strongest = strength2 > strengths[strongest] ? 1 : strongest;
    strongest = strength3 > strengths[strongest] ? 2 : strongest;
    strongest = strength4 > strengths[strongest] ? 3 : strongest;
    
    int centerdest = strength1 > 5 ? 0 : 4;
    centerdest = abs(reading2) < abs(readings[centerdest]) && strength2 > 5 ? 1 : centerdest;
    centerdest = abs(reading3) < abs(readings[centerdest]) && strength3 > 5 ? 2 : centerdest;
    centerdest = abs(reading4) < abs(readings[centerdest]) && strength4 > 5 ? 3 : centerdest;

    if (centerdest == 4) {
      centerdest = abs(reading2) < abs(readings[centerdest]) && strength2 > 0 ? 1 : centerdest;
      centerdest = abs(reading3) < abs(readings[centerdest]) && strength3 > 0 ? 2 : centerdest;
      centerdest = abs(reading4) < abs(readings[centerdest]) && strength4 > 0 ? 3 : centerdest;
    }
    
//    display.clearDisplay();
//    line1(String(centerdest + 1));
//    line2(String(readings[centerdest]));
    sprintf(buffer, "\n[ %i , %i ]  [ %i , %i ]  [ %i , %i ]  [ %i , %i ]", reading1, strength1, reading2, strength2, reading3, strength3, reading4, strength4);
    Serial.print(buffer);

    if (retStrength) {
      return strengths[centerdest];
    }

    int  angLookup[4][5] = {
      {81, 63, 45, 27, 9},
      {-9, -27, -45, -63, -81},
      {-99, -117, -135, -153, -171},
      {171, 153, 135, 117, 99}
    };

    if (readings[centerdest] > 2) {
      int newCenterdest = centerdest + 1;
      int newReading = readings[centerdest] - 2;
      if (newCenterdest > 3 || newCenterdest < 0 || newReading > 4 || newReading < 0) {
        return 0;
      }
      return -angLookup[newCenterdest][newReading];
    } else if (readings[centerdest] < -2) {
      int newCenterdest = centerdest - 1;
      int newReading = readings[centerdest] + 6;
      if (newCenterdest > 3 || newCenterdest < 0 || newReading > 4 || newReading < 0) {
        return 0;
      }
      return -angLookup[newCenterdest][newReading];
    }

//    display.clearDisplay();
    //line1(String(-angLookup[centerdest][readings[centerdest] + 2]));
//    line1(String(readings[centerdest]));
//    line2(String(strengths[centerdest]));
//    line3(String(centerdest));
    
    return -angLookup[centerdest][readings[centerdest] + 2];
  }

}

