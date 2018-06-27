# laser-snake-temp-control
On-off temperature control for a home brew fermentation system, with Arduino. 

# Motivation
To improve brew product quality, our home-brew system needed more precise temperture control during fermentation. With careful tuning, this simple on-off control can maintain the fermentation temperature of a 23L brew to within 0.2 degrees celcius of the set temperature. The fermentation vesel is placed inside a fridge, which we line with a heated cable. Relays are connected to the fridge and the heated cable. The temperature control is based on two temperature measurements, one measuring the ambient temperature of the fridge, and one inside the fermentation vesel. 

# Getting Started 

## Arduino libraries
<ul>
  <li> Paul Stoffregen's <a href="https://github.com/PaulStoffregen/OneWire">OneWire</a> </li>
  <li> Miles Burton's <a href="https://github.com/milesburton/Arduino-Temperature-Control-Library">DallasTemperature</a></li>
  <li> The <a ref="https://playground.arduino.cc/Main/RunningMedian">running median Arduino library</a></li>
</ul>

## Materials needed
 <ul>
   <li> Arduino :) </li>
   <li> Two one-wire digital temperature sensors </li>
   <li> Two arduino relays </li>
 </ul>

## Schema
