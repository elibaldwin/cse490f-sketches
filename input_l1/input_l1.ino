const int INPUT_1 = 3;
const int INPUT_2 = 4;
const int INPUT_3 = 5;

const int OUT_1 = 11;
const int OUT_2 = 12;
const int OUT_3 = 13;

void setup()
{
  pinMode(INPUT_1, INPUT); // external pull-up resistor
  pinMode(INPUT_2, INPUT); // external pull-down resistor
  pinMode(INPUT_3, INPUT_PULLUP);

  pinMode(OUT_1, OUTPUT);
  pinMode(OUT_2, OUTPUT);
  pinMode(OUT_3, OUTPUT);
  Serial.begin(9600); // for printing values to console
}

void loop()
{ 
  digitalWrite(OUT_1, 1 - digitalRead(INPUT_1));
  digitalWrite(OUT_2, digitalRead(INPUT_2));
  digitalWrite(OUT_3, 1 - digitalRead(INPUT_3));
  
  delay(5);                                      // small delay
}
