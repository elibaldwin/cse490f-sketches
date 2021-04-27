const int POT_PIN = A0;
const int BUF_SIZE = 16;

class RingBuffer {
  private:
    int buf[BUF_SIZE];
    int sum;
    int i;

  public:
    int update_buf(int val) {
      sum -= buf[i];
      sum += val;
      buf[i] = val;
      i = (i+1) % BUF_SIZE;
      return sum / BUF_SIZE;
    }
};

RingBuffer smooth_buf;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  int pot_val = smooth_buf.update_buf(analogRead(POT_PIN));
  Serial.println(pot_val);

  delay(5);
}
