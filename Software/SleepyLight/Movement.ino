SF fusion;

float gx, gy, gz, ax, ay, az;
float pitch, roll, yaw;
float deltat;

float pPitch, pRoll;

#define TURN_THRESHOLD 10

unsigned long last;

boolean detectTurn(Orientation lastOrientation) {
  Orientation currentOrientation;
  
  if (pitch < 0) {
    currentOrientation = UPWARD;
  } else if (pitch > 0) {
    currentOrientation = DOWNWARD;
  }
  orientation = currentOrientation;
  
  if (lastOrientation != NONE) {
    return (currentOrientation != lastOrientation);
  } else {
    return false;
  }
}

unsigned long shakeDebounce;
unsigned long lastShake;

#define SHAKE_SENSITIVITY 4
#define SHAKE_RESET 2000

void detectMovement() {
  if (roll - pRoll > SHAKE_SENSITIVITY || pitch - pPitch > SHAKE_SENSITIVITY) {
    if (millis() - shakeDebounce > 75) {
      Serial.println("--shake--");
      nShakes++;
      lastShake = millis();
      shakeDebounce = millis();
      client.publish("/shakes", String(nShakes));
    }
    
//    delay(75);
  }
  if (millis() - lastShake > SHAKE_RESET) {
    nShakes = 0;
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
