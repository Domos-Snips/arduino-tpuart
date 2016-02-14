Arduino EIB/KNX Interface via TP-UART
=====================================


This is a very first version of an interface between Arduino and EIB/KNX using the TP-UART interface and Arduino library.


Hardware
--------

We leverage Siemens BCU 5WG1117-2AB12 - for about 30€

Software
--------

The Arduino library is found in the directory `KnxTpUart` and can be directly placed in Arduino's library folder. 


Issues, Comments and Suggestions
--------------------------------

If you have any, don't hesitate to contact me or use the issue tracker. You are also invited to improve the code and send me a pull request to reintegrate the changes here.

Supported commands / telegram types
-----------------------------------

Bool (DPT 1 - 0 or 1)

knx.groupWriteBool("1/2/3", bool);



2 ByteFloat (DPT9 - -671 088,64 to 670 760,96 )

knx.groupWrite2ByteFloat("1/2/3", float);



1 Byte Int (DTP 5 - 0...255)

knx.groupWrite1ByteInt("1/2/3", int);



2 byte Int (DTP 7 - 0…65 535])

knx.groupWrite2ByteInt("1/2/3", int);



4 byte Float (DTP 14 - -2147483648 to 2147483647) 

knx.groupWrite4ByteFloat("1/2/3", float);



14 Byte text (DTP 16)

knx.groupWrite14ByteText("1/2/3", String);



knx.Time (DTP 10.001)

groupWriteTime("1/2/3", Day, Hours, Minutes, Seconds);



Also all answer requests (except Time):
---------------------------------------

knx.groupAnswerBool("1/2/3", bool);

knx.groupAnswer2ByteFloat("1/2/3", float);

knx.groupAnswer1ByteInt("1/2/3", int);

knx.groupAnswer2ByteInt("1/2/3", int);

knx.groupAnswer4ByteFloat("1/2/3", float);

knx.groupAnswer14ByteText("1/2/3", String);


Also all read requests (except Time) - (see example groupRead):
---------------------------------------------------------------

value = telegram->getBool();

value = telegram->get1ByteIntValue();

value = telegram->get2ByteIntValue();

value = telegram->get2ByteFloatValue();

value = telegram->get4ByteFloatValue();

value = telegram->get14ByteValue();