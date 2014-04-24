rpio
====

raspberry pi / eurorack stereo DAC breakout board

ADC and FUDI message handling for interfacing with puredata.

cf muffwiggler.com for more details:

http://www.muffwiggler.com/forum/viewtopic.php?t=104896&postdays=0&postorder=asc&start=0

also see https://github.com/constantin3000/PiCollider for using with supercollider



adc2FUDI.py  python script
adc2FUDI.c   the same, in c

usage:

both scripts/programs send "FUDI" messages to pd, so expect a TCP port to be open [ = suitable pd patch running].

the messages are prefixed with an id 0-10, where
0  ADC1
1  ADC2
...
5  ADC6
6  ClkIn 1
7  ClkIn 2
8  ClkIn 3
9  button 1
10 button 2


so ... eg:

[netreceive 54321]
|
[route 0 1 2 3 4 5 6 7 8 9 10]
| | | | | | | | | | 
[etc]

=======================================

useage:

python:

python ADC2pd.py


C:

compile it (wiringPi needs to be installed): 

gcc adc2FUDI.c -lwiringPi -std=c99 -Wall -o adc2FUDI 

run it: 

adc2FUDI <IP address> <port number> <update rate in ms> 

eg:

sudo ./adc2FUDI 127.0.0.1 54321 10 & 

