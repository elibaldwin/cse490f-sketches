
// Simple ring buffer implementation, using a template
// to allow custom buffer sizes

// (higher values of N will result in better smoothing, but also
// increase response time for changing values)

template <int N>
class RingBuffer {
  private:
    int buf[N];
    long sum;  // running sum of all buffer values
    int i;     // 'pointer' to next slot in buffer

  public:
    int update_buf(int val) {
      sum -= buf[i];  // subtract the value in the next slot from the sum
      sum += val;     // add the new value to the sum
      buf[i] = val;   // put the new value in the next slot
      i = (i+1) % N;  // increment slot pointer
      return sum / N; // return average of all buffer values
    }

    int get() {
      return sum / N;
    }
};
