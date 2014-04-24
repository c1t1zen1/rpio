rpio
====

raspberry pi / eurorack stereo DAC breakout board

ADC and FUDI message handling (betabeta) for interfacing with puredata: http://en.wikipedia.org/wiki/FUDI

cf. muffwiggler.com for more details:

http://www.muffwiggler.com/forum/viewtopic.php?t=104896&postdays=0&postorder=asc&start=0

also see https://github.com/constantin3000/PiCollider for using with supercollider (also re using OSC)

================================================================

adc2FUDI.py  :::  python script<br>
adc2FUDI.c   :::  same thing, in c<br>

both scripts/programs send "FUDI" messages to pd, thus expect a TCP port to be open [ = suitable pd patch running].
<br>
<br>
the messages are prefixed with an id 0-10, where<br>
0  ADC1       [12bit]<br>
1  ADC2       [12bit]<br>
...<br>
5  ADC6       [12bit]<br>
6  ClkIn 1    [1bit] <br>
7  ClkIn 2    [1bit]<br>
8  ClkIn 3    [1bit]<br>
9  button 1   [1bit]<br>
10 button 2   [1bit]<br>


so ... in pd you'd do, for example:<br>

[netreceive 54321]<br>
|<br>
[route 0 1 2 3 4 5 6 7 8 9 10]<br>
| | | | | | | | | | <br>
[...]<br>

=======================================================================

usage ----

python (requires spidev):

python ADC2pd.py &


C:

compile it (requires wiringPi): <br>

gcc adc2FUDI.c -lwiringPi -std=c99 -Wall -o adc2FUDI<br> 

run it: <br>

adc2FUDI  *IP-address*  *port-number*  *update-rate-in-ms* 

eg:

sudo ./adc2FUDI 127.0.0.1 54321 10 & 

