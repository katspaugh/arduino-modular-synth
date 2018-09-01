# Euclidean clock divider

Takes a clock signal as input, and outputs Euclidean rhythms.

The yellow button switches the amount of active beats there is in a pattern (from 1 out of 8 to 8 out of 8).

The jacks, from top left to bottom right:

* Input 1: takes a clock/gate singal to progress the sequence.
* Input 2: a reset signal. Restarts the sequence when high.
* Output 1: inversed beats. Sends a trigger on every non-active beat.
* Output 2: active beats, the actual Euclidean rhythm.

The input impedance is 100k Ohm, the outputs are 10k Ohm.

There's no diode protection on any of the jacks, so only positive voltages please.

## Schematic

<img width="951" src="https://user-images.githubusercontent.com/381895/44620923-73be8e00-a89d-11e8-91a2-1c68c89ee5cd.png">

The LEDs may be connected to any digital pins in any order (except the ones used by the jacks).
Please modify the pins in the code according yo your wiring.

## Photo
<img width="800" alt="photo" src="https://user-images.githubusercontent.com/381895/40321318-ef1b6f1a-5d2e-11e8-9602-681e281e2991.jpeg">

## Video demo
[A demo of the module on Vimeo](https://vimeo.com/286662190).
