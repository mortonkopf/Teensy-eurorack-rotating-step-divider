# Teensy-eurorack-rotating-step-divider
simple sketch for using a Teensy 3.* as a eurorack rotating step divider. you can change your step dic values in the code

this sketch setsup a teensy 3.* to act as a step divider. it countstheincoming analogue signal clocks, and then sends out clock pulses on the seven outputs at the given count. These values can be changed (eg, use odd number divisions for interesting beats) in the code by alteringhte divider value in the array.

