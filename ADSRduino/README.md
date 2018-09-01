# ADSRduino
This is a simple ADSR envelope generator for the Arduino

It is a hardware generator; it exists to generate a physical envelope voltage, which is output from an MCP4921 DAC converter
(unlike code to generate soft signals to control software synths etc).

You can find some description at http://m0xpd.blogspot.co.uk/2017/02/signal-processing-on-arduino.html

## Modifications

@katspaugh modified this sketch to use the [Wire](https://www.arduino.cc/en/Reference/Wire) library
for the communication with the DAC.
