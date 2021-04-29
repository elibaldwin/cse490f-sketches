
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
