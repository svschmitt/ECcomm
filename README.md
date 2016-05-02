# ECcomm
This is a Linux program that communicates with a microcontroller equipped with the ECcomm_adapter code (https://github.com/svschmitt/EC-PCB/ ). Together, these items allow for bidirectional communication with the Entertainment &amp; Comfort serial bus found in some GM vehicles. For more information, see http://stuartschmitt.com/e_and_c_bus/

To compile, simply run "make".

To run the program, supply as an argument the serial device for the Arduino. For example,
  ./ECcomm /dev/ttyACM0

Operating instructions are supplied in the program. One useful feature is E&C commands mapped to keystrokes. These are contained in the files .ECcomm in the user's home directory and ECcomm.cfg in the present working directory. A sample ECcomm.cfg file is supplied.

Better documentation is on the to-do list.
