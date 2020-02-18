# Introduction
## Strategy
### Circuit switching
Waste resource if the connection is established by not in use.
### Packet switching
### Sychronous Time Multiplexing & Frequency Multiplexing
Not scalable
Waste resource when allocated time slot/ frequency is not in use
### Statistical Multiplexing
- Used in Internet
## Addressing
Byte string that identifies a node
Type of addresses
- unique: node-specific
- broadcast: all nodes on the network
- multicast: some subset of nodes on the network
## Forwarding
A local term. Get packet from one of its port, and decide which port to send.
## Routing
## Performance
- Latency
  - initial time to set up communication (setup time)
  - time takes to put data block on link (transimit time)
    ```data size/bandwith```
  - link delay (propagation delay)
    ```=link distance/c(light speed)```
  - router delay (queuing delay)
  - 
- Throughput
  ```=size of data block/latency```

A trap: 
- File Size in capital ``B``, such as: ``KB, MB, GB`` is based 2
- bandwidth in lower case ``b``, such as ``Mbps, Gbps`` is based 10.
Example:

# Link
## Problems
### Signal deteriation
Over a distance, the signal itself can deteriate. -> addaptive threshold (lower threshold when receiving 0) for a low voltage to be recognized as a 0
There can be noises, so that a sqaure wave is not a square wave anymore (guigle)
If send consecutive 1s and 0s, the threshold may be too low that any white noise would causing a wrong bits.
### Synchronization
Receiver and sender may not well-syncrhonized, so that the sampled signal is out of clock and wrong.
#### Manchester
#### Sentinal-based
#### Clock-based
### Error
Bit-level errors (caused by interference)  
Error detection probability <-> Overhead
#### Detection
Through addition of redundant information to frame.  
Error detection code is **overhead**, without carrying any data
- Parity
- CRC(Cyclic Redundancy Check)
  - Add ``k`` bits of redundant data to an ``n`` bits message, when ``k << n``
  - Represent n-bit message as n-1 degree polynomial
    - eg. MESSAGE=10011010 as M(x)=x^7+x^4+x^3+x^1
  - Let k be the degree of some divisor polynomial
    - eg. C(x)=x^3+x^2+1
  - Transimit polynomial P(x) that is evenly divisible by C(x)
    - shift left k bits, ie. M(x)*x^k
    - subtract the remainder of ``M(x)*x^k/C(x)`` from ``M(x)*x^k``, ie. P(x)=M(x)*x^k-[remainder of M(x)*x^k/C(x) ]
  - hardware implementation: feedback shift register
- Checksum
  + cannot be implemented efficiently by hardware (binary addition, implemented as software programs)
  + 
## Reliable Transimition
### Stop and Wait Algorithm
- After transimited 1 frame, the Sender(S) waits for ACK before transimitting the next frame
- If time out, retransimit  
Problem: 
1. duplicate frames
2. The sequence number may also get flipped, causing errors
3. The bandwidth utilization is low.
### Sliding Window
![SW:Sender](./images/1.png)
![SW:Receiver](./images/2.png)