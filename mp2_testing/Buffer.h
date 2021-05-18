
/*  Elias Baldwin
 *  CSE490 Spring 2021
 *  MP2: Microgame
 *  
 *  Buffer.h
 *  
 *  This is a utility class that I made, pretty much just used to keep
 *  track of the different platforms that are generated and rendered
 *  for the Jump! game.
 * 
 */

template<class T, int N>
class Buffer {
  private:
    T a[N];
    int front = 0;
    int back = 0;
    int n = 0;

  public:    
    void push_back(T item) {
      if (n < N) {
        a[back] = item;
        back = (back+1) % N;
        n++;
      }
    }

    T pop_front() {
      T res = a[front];
      front = (front + 1) % N;
      n--;
      return res;
    }

    // overload the subscript operator for accessing internal buffer elements
    T &operator[](int ind) {
      return a[(front + ind) % N];
    }

    int size() {
      return n;
    }

    void clear() {
      n = 0;
      front = 0;
      back = 0;
    }
};
