#Requirements

Design a "laser maze" type system composed out of 20 laser/detector pairs

* Green pointer lasers;
* Battery operated - 3.7V 18650 li-ion;
* More than 10h operation on a single charge;
* Wireless at 433MHz;
* Latency up to 5ms;
* Vibration tolerant sensor;

#Sensor design

To avoid false positives caused by vibration of the sensor or the laser, we decided to use a sensing area of 50mm in diameter. Sensor recepticle was cut out of diffusing plexiglass. We expected that laser light dissipation will be enough for the whole diffusor to glow. 

We've chosen TEMT6200FX01 phototransistor as a sensing element because of it's sensitivity to green light and wide angle. 

Test rig was designed in openscad and 3D printed. 

![testrig design](pics/sensor-testrig-3d.png)
![testrig photo](pics/DSC01677.JPG)

During testing it became clear that the plexiglass does not diffuse the laser light enough for single photo-transistor to detect uniformly over the whole area. Also, sensitivity of the sensor was too low - there is a tradeoff - higher the load resistor, higher the sensitivity, slower the response. We wanted to keep the response time of the sensor under 1ms, this limited the sensitivity we could achieve thus we had to use a comparator for beam break detection.  

Soon we have realised that a single photo-transistor won't work. So we have moved to 4 photottransistor grid.
![test grid](pics/DSC01680.JPG)

This grid gave us pretty even signal over the whole are of the sensor, difference between illuminated and non illuminated reading being ~100mV using red pointer laser.

As ambient light influences the idle level of the sensor a lot, we needed a means to perform calibration. We decided to use a digital potentiometer for that.

#Radio
We expected to use a cheap ebay 433MHz transmitter/receiver pair for radio communication, but at the first test it proved to be insufficient. The communication was unreliable and too slow at 4800 baud rate. So we decided to test RFM01/RFM02 pair by Hope electronics.

#Lasers

We have decided to use green laser pointers as a laser module source as it was the cheapest option. 

![](pics/s-l400.jpg)

During the member day meetup event in the Technarium hackerspace we took some time to peel the aluminium case off the modules - some mental echo of ancient people sitting around the fire and peeling fruits. Modules proved to be of several kinds physically, but all of them used the same constant current driver circuit.
