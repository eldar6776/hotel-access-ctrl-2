@ECHO OFF
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\rcbl.hex -intel -offset -0x8000000 -crop 0 0x2FF0 -Exclusive_Length_Little_Endian 0x2FF0 -fill 0xFF 0 0x2FF0 -o Exe\tmp1.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp1.hex -intel -crop 0 0x2000 0x2008 0x2FF0 Exe\tmp1.hex -intel -crop 0x2FF0 0x2FF8 -offset -0xFF0 -o Exe\tmp2.hex -intel 
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp2.hex -intel -unfill 0xFF 4 -o Exe\tmp3.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp3.hex -intel -fill 0xFF -over Exe\tmp3.hex -intel -o Exe\tmp4.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp4.hex -intel -STM32 0x2FF4 -crop 0x2FF4 0x2FF8 -offset -0xFF0 -o Exe\tmp5.hex -intel 
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp4.hex -intel -exclude 0x2004 0x2008 Exe\tmp5.hex -intel -o Exe\tmp6.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp6.hex -intel -o Out\rcbl.bin -binary
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp6.hex -intel -offset 0x08000000 -o Out\rcbl.hex -intel
exit