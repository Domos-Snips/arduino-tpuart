Arduino EIB/KNX Interface via TP-UART
=====================================


This is a very first version of an interface between Arduino and EIB/KNX using the TP-UART interface and Arduino library.


Hardware
--------

We leverage Siemens BCU 5WG1 117-2AB12 - for about ~30�


We leverage ARDUINO MEGA (SERIAL PORT 1)

-> No restrictions on the programming


We leverage ARDUINO UNO (SERIAL PORT)

-> During programming, the BCU may have no connection to the ARDUINO UNO

-> After programming for communication ICSP 1 PIN 5 and ICSP PIN 6 connect, tested only with ARDUINO UNO revision 3


Software
--------

The Arduino library is found in the directory `KnxTpUart` and can be directly placed in Arduino's library folder. 


Issues, Comments and Suggestions
--------------------------------

If you have any, don't hesitate to contact me or use the issue tracker. You are also invited to improve the code and send me a pull request to reintegrate the changes here.


Supported commands / telegram types
-----------------------------------
All commando (write) to bus :
-----------------------------

Bool (DPT 1 - 0 or 1)

knx.groupWriteBool("1/2/3", bool);



4 Bit Int (DPT 3)

knx.groupWrite4BitInt("1/2/3", int);



4 Bit Dim (DPT 3)

knx.groupWrite4BitDim("1/2/3", direction, steps);



1 Byte Int (DTP 5 - 0...255)

knx.groupWrite1ByteInt("1/2/3", int);



2 Byte Int (DTP 7 - 0�65 535])

knx.groupWrite2ByteInt("1/2/3", int);



2 Byte Float (DPT9 - -671 088,64 to 670 760,96 )

knx.groupWrite2ByteFloat("1/2/3", float);



3 Byte Time (DTP 10)

groupWrite3ByteTime("1/2/3", Weekday, Hour, Minute, Second);



3 Byte Date (DTP 11)

groupWrite3ByteDate("1/2/3", Day, Month, Year);



4 byte Float (DTP 14 - -2147483648 to 2147483647) 

knx.groupWrite4ByteFloat("1/2/3", float);



14 Byte Text (DTP 16)

knx.groupWrite14ByteText("1/2/3", String);



All commando (answer) to bus :
------------------------------

Bool (DPT 1 - 0 or 1)

knx.groupAnswerBool("1/2/3", bool);



4 Bit Int (DPT 3)

commented out -> knx.groupAnswer4BitInt("1/2/3", int);



4 Bit Int (DPT 3)

commented out -> knx.groupWrite4BitDim("1/2/3", bool, byte);



1 Byte Int (DTP 5 - 0...255)

knx.groupAnswer1ByteInt("1/2/3", int);



2 Byte Int (DTP 7 - 0�65 535])

knx.groupAnswer2ByteInt("1/2/3", int);



2 Byte Float (DPT9 - -671 088,64 to 670 760,96 )

knx.groupAnswer2ByteFloat("1/2/3", float);



3 Byte Time (DTP 10)

knx.groupAnswer3ByteTime("1/2/3", int, int, int, int);



3 Byte Date (DTP 11)

knx.groupAnswer3ByteDate("1/2/3", int, int, int);



4 byte Float (DTP 14 - -2147483648 to 2147483647)

knx.groupAnswer4ByteFloat("1/2/3", float);



14 Byte Text (DTP 16)

knx.groupAnswer14ByteText("1/2/3", String);


Once for all DTP -> commando (read) to bus :
--------------------------------------------

knx.groupRead("1/2/3");



Evaluation telegrams from bus :
-------------------------------

Bool (DPT 1 - 0 or 1)

value = telegram->getBool();



4 Bit Int (DPT 3)

value = telegram->get4BitIntValue();



Bool (DPT 3)

value = telegram->get4BitDirectionValue();



1 byte (DPT 3)

value = telegram->get4BitStepsValue();



1 Byte Int (DTP 5 - 0...255)

value = telegram->get1ByteIntValue();



2 Byte Int (DTP 7 - 0�65 535])

value = telegram->get2ByteIntValue();



2 Byte Float (DPT9 - -671 088,64 to 670 760,96 )

value = telegram->get2ByteFloatValue();



3 Byte Time (DTP 10)

value = telegram->get3ByteWeekdayValue();



3 Byte Time (DTP 10)

value = telegram->get3ByteHourValue();



3 Byte Time (DTP 10)

value = telegram->get3ByteMinuteValue();



3 Byte Time (DTP 10)

value = telegram->get3ByteSecondValue();



3 Byte Time (DTP 11)

value = telegram->get3ByteDayValue();


3 Byte Time (DTP 11)

value = telegram->get3ByteMonthValue();



3 Byte Time (DTP 11)

value = telegram->get3ByteYearValue();



4 byte Float (DTP 14 - -2147483648 to 2147483647)

value = telegram->get4ByteFloatValue();



14 Byte Text (DTP 16)

value = telegram->get14ByteValue();