SF fusion;

float gx, gy, gz, ax, ay, az;
float pitch, roll, yaw;
float deltat;

float pPitch, pRoll;

int nShakes;

boolean detectTurn(Orientation lastOrientation) {
  Orientation currentOrientation;
  
  if (pitch < -60) {
    currentOrientation = UPWARD;
  } else if (pitch > 60) {
    currentOrientation = DOWNWARD;
  }
  orientation = currentOrientation;
  return (currentOrientation != lastOrientation);
}

unsigned long shakeDebounce;

void detectMovement() {
  if (roll - pRoll > 4) {
    if (millis() - shakeDebounce > 50) {
      Serial.println("--shake--");
      nShakes++;
      Serial.println(nShakes);
      shakeDebounce = millis();
    }
    
//    delay(75);
  }
}

void updateOrientation() {
  pPitch = pitch;
  pRoll = roll;

  
  
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    float gyroScale = 0.097656f / 4.5;
    gx = gx * gyroScale;
    gy = gy * gyroScale;
    gz = gz * gyroScale;    
  }
  deltat = fusion.deltatUpdate(); //this have to be done before calling the fusion update
  fusion.MahonyUpdate(gx, gy, gz, ax, ay, az, deltat);

  pitch = fusion.getPitch();
  roll = fusion.getRoll();
  yaw = fusion.getYaw();

//  Serial.print(pitch - pPitch);
//  Serial.print(" ");
//  Serial.println(roll - pRoll);
}
