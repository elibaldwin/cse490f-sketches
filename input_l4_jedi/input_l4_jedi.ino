const int FSR_PIN = A0;
//const int LED_PIN = 3;
const int PIEZO_PIN = 3;
const int DELAY = 200;

const int N_NOTES = 8;
int NOTES[] = {262, 294, 330, 349, 392, 440, 494, 523};


void setup() {
  // put your setup code here, to run once:
  pinMode(PIEZO_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int fsrVal = analogRead(FSR_PIN);
  Serial.print("fsr:");
  Serial.print(fsrVal);
  
  if (fsrVal > 300) {
    int minDist = 1000;
    int note = 0;
    long mappedFsrVal = logmap(fsrVal-300, 0, 700, NOTES[0]-15, NOTES[N_NOTES-1]);
    Serial.print(" mapped:");
    Serial.print(mappedFsrVal);
    for (int i = 0; i < N_NOTES; i++) {
      long dist = abs(mappedFsrVal - NOTES[i]);
      if (dist < minDist) {
        minDist = dist;
        note = NOTES[i];
      }
    }
    tone(PIEZO_PIN, note);
  } else {
    noTone(PIEZO_PIN);
  }
  
  Serial.println();  

  delay(DELAY);
}


long logmap(long val, long min1, long max1, long min2, long max2) {
  long pval = val * val;
  long pmin1 = min1 * min1;
  long pmin2 = max1 * max1;
  Serial.print("pval: ");
  Serial.print(pval);
  Serial.print(" pmin1: ");
  Serial.print(pmin1);
  Serial.print(" pmin2: ");
  Serial.println(pmin2);
  return map(pval, pmin1, pmin2, min2, max2);
}
