@ECHO OFF
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\rc.hex -intel -offset -0x8003000 -crop 0 0xCFF0 -Exclusive_Length_Little_Endian 0xCFF0 -fill 0xFF 0 0xCFF0 -o Exe\tmp1.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp1.hex -intel -crop 0 0x2000 0x2008 0xCFF0 Exe\tmp1.hex -intel -crop 0xCFF0 0xCFF8 -offset -0xAFF0 -o Exe\tmp2.hex -intel 
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp2.hex -intel -unfill 0xFF 4 -o Exe\tmp3.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp3.hex -intel -fill 0xFF -over Exe\tmp3.hex -intel -o Exe\tmp4.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp4.hex -intel -STM32 0xCFF4 -crop 0xCFF4 0xCFF8 -offset -0xAFF0 -o Exe\tmp5.hex -intel 
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp4.hex -intel -exclude 0x2004 0x2008 Exe\tmp5.hex -intel -o Exe\tmp6.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp6.hex -intel -o Out\rc.bin -binary
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\tmp6.hex -intel  -offset 0x08003000 -o Out\rc.hex -intel
C:\Keil\ARM\Utilities\srecord\srec_cat Exe\rc.hex -intel ..\..\RCBL\MDK-ARM\Out\rcbl.hex -intel -o Out\rc_all_in_one.hex -intel
exit