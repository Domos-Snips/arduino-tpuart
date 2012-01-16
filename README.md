Arduino EIB/KNX Interface via TP-UART
=====================================


This is a very first version of an interface between Arduino and EIB/KNX using the TP-UART interface IC. It consists of the hardware and an Arduino library.


Hardware
--------

The schematic is found in the directory `Eagle-Files`. I didn't create a PCB layout (yet) - please let me know if you are interested). The schematic is largely based on the TPUART board designed at the [University of Vienna](http://www.auto.tuwien.ac.at/~mkoegler/index.php/tpuart).


TP-UART ICs can be obtained from [Opternus Components](http://www.opternus.com/de/siemens/knx-chipset/knx-transceiver-ics-tp-uart-fze1066.html).


Software
--------

The Arduino library is found in the directory `KnxTpUart` and can be directly placed in Arduino's library folder. It is currently in a very early alpha state and undergoes constant change. I recommend not to trust on it as of now.


Issues, Comments and Suggestions
--------------------------------

If you have any, don't hesitate to contact me or use the issue tracker. You are also invited to improve the code and send me a pull request to reintegrate the changes here.

