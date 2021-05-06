# MP1: Night Light: The Code
(yes, I know it's called `mp1_testing`...)

The main bit is in `mp1_testing.ino`. I used the PushButton and RGBConverter libraries that we saw in the Arduino I/O exercises in class. 
I also wrote my own class, called `RingBuffer`, contained entirely within `RingBuffer.h` (it's simple enough that I think it can survive not being broken into two files). 
`RingBuffer` is my own simple implementation of a ring buffer used for smoothing noisy input values.
