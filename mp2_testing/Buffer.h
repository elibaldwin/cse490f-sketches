
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
 *  The basic idea is to have a buffer of fixed maximum size, with 
 *  constant time push_back and pop_front operations, as well
 *  as constant time indexing. This allows adding platforms to the back
 *  and removing them from the front. It's also still relatively easy
 *  to iterate through the structure using the overloaded subscript operator.
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
