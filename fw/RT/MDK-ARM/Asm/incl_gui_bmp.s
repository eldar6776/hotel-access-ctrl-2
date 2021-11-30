	AREA    gui_bmp, DATA, READONLY
	EXPORT  disp_00
	EXPORT  disp_01
	EXPORT  disp_02
	EXPORT  disp_03
	EXPORT  disp_04
	EXPORT  disp_05
	EXPORT  disp_10
	EXPORT  disp_11
	EXPORT  disp_12
	EXPORT  disp_13
	EXPORT  disp_14
	EXPORT  disp_15
	EXPORT  disp_21
	EXPORT  disp_22
	EXPORT  disp_23
	EXPORT  disp_24
	EXPORT  disp_25
	EXPORT  disp_61
	EXPORT  disp_62
	EXPORT  disp_63
	EXPORT  disp_64
	EXPORT  disp_65
	EXPORT  disp_71
	EXPORT  disp_72
	EXPORT  disp_73
	EXPORT  disp_74
	EXPORT  disp_75            
	;EXPORT  disp_81
	;EXPORT  disp_82
	;EXPORT  disp_83
	;EXPORT  disp_84
	;EXPORT  disp_85
	EXPORT imgBackground
	  
disp_00
	INCBIN  ../Src/Display/display_00.bmp
	ALIGN
disp_00_end
disp_00_size
	DCD     disp_00_end - disp_00
		
disp_01
	INCBIN  ../Src/Display/display_01.bmp
	ALIGN
disp_01_end
disp_01_size
	DCD     disp_01_end - disp_01

disp_02
	INCBIN  ../Src/Display/display_02.bmp
	ALIGN
disp_02_end
disp_02_size
	DCD     disp_02_end - disp_02
	
disp_03
	INCBIN  ../Src/Display/display_03.bmp
	ALIGN
disp_03_end
disp_03_size
	DCD     disp_03_end - disp_03
		
disp_04
	INCBIN  ../Src/Display/display_04.bmp
	ALIGN
disp_04_end
disp_04_size
	DCD     disp_04_end - disp_04
	
disp_05
	INCBIN  ../Src/Display/display_05.bmp
	ALIGN
disp_05_end
disp_05_size
	DCD     disp_05_end - disp_05

disp_10
	INCBIN  ../Src/Display/display_10.bmp
	ALIGN
disp_10_end
disp_10_size
	DCD     disp_10_end - disp_10
	
disp_11
	INCBIN  ../Src/Display/display_11.bmp
	ALIGN
disp_11_end
disp_11_size
	DCD     disp_11_end - disp_11

disp_12
	INCBIN  ../Src/Display/display_12.bmp
	ALIGN
disp_12_end
disp_12_size
	DCD     disp_12_end - disp_12
	
disp_13
	INCBIN  ../Src/Display/display_13.bmp
	ALIGN
disp_13_end
disp_13_size
	DCD     disp_13_end - disp_13
	
disp_14
	INCBIN  ../Src/Display/display_14.bmp
	ALIGN
disp_14_end
disp_14_size
	DCD     disp_14_end - disp_14
	
disp_15
	INCBIN  ../Src/Display/display_15.bmp
	ALIGN
disp_15_end
disp_15_size
	DCD     disp_15_end - disp_15
	
disp_21
	INCBIN  ../Src/Display/display_21.bmp
	ALIGN
disp_21_end
disp_21_size
	DCD     disp_21_end - disp_21
	
disp_22
	INCBIN  ../Src/Display/display_22.bmp
	ALIGN
disp_22_end
disp_22_size
	DCD     disp_22_end - disp_22
	
disp_23
	INCBIN  ../Src/Display/display_23.bmp
	ALIGN
disp_23_end
disp_23_size
	DCD     disp_23_end - disp_23
	
disp_24
	INCBIN  ../Src/Display/display_24.bmp
	ALIGN
disp_24_end
disp_24_size
	DCD     disp_24_end - disp_24
	
disp_25
	INCBIN  ../Src/Display/display_25.bmp
	ALIGN
disp_25_end
disp_25_size
	DCD     disp_25_end - disp_25
        
disp_61
	INCBIN  ../Src/Display/display_61.bmp
	ALIGN
disp_61_end
disp_61_size
	DCD     disp_61_end - disp_61

disp_62
	INCBIN  ../Src/Display/display_62.bmp
	ALIGN
disp_62_end
disp_62_size
	DCD     disp_62_end - disp_62

disp_63
	INCBIN  ../Src/Display/display_63.bmp
	ALIGN
disp_63_end
disp_63_size
	DCD     disp_63_end - disp_63

disp_64
	INCBIN  ../Src/Display/display_64.bmp
	ALIGN
disp_64_end
disp_64_size
	DCD     disp_64_end - disp_64

disp_65
	INCBIN  ../Src/Display/display_65.bmp
	ALIGN
disp_65_end
disp_65_size
	DCD     disp_65_end - disp_65

disp_71
	INCBIN  ../Src/Display/display_71.bmp
	ALIGN
disp_71_end
disp_71_size
	DCD     disp_71_end - disp_71

disp_72
	INCBIN  ../Src/Display/display_72.bmp
	ALIGN
disp_72_end
disp_72_size
	DCD     disp_72_end - disp_72

disp_73
	INCBIN  ../Src/Display/display_73.bmp
	ALIGN
disp_73_end
disp_73_size
	DCD     disp_73_end - disp_73
		
disp_74
	INCBIN  ../Src/Display/display_74.bmp
	ALIGN
disp_74_end
disp_74_size
	DCD     disp_74_end - disp_74

disp_75
	INCBIN  ../Src/Display/display_75.bmp
	ALIGN
disp_75_end
disp_75_size
	DCD     disp_75_end - disp_75

;disp_81
;	INCBIN  ../Src/Display/display_81.bmp
;	ALIGN
;disp_81_end
;disp_81_size
;	DCD     disp_81_end - disp_81
;		
;disp_82
;	INCBIN  ../Src/Display/display_82.bmp
;	ALIGN
;disp_82_end
;disp_82_size
;	DCD     disp_82_end - disp_82

;disp_83
;	INCBIN  ../Src/Display/display_83.bmp
;	ALIGN
;disp_83_end
;disp_83_size
;	DCD     disp_83_end - disp_83

;disp_84
;	INCBIN  ../Src/Display/display_84.bmp
;	ALIGN
;disp_84_end
;disp_84_size
;	DCD     disp_84_end - disp_84

;disp_85
;	INCBIN  ../Src/Display/display_85.bmp
;	ALIGN
;disp_85_end
;disp_85_size
;	DCD     disp_85_end - disp_85

imgBackground
	INCBIN  ../Src/Display/imgBackground.bmp
	ALIGN
imgBackground_end
imgBackground_size
	DCD     imgBackground_end - imgBackground
    
    
	EXPORT  disp_00_size            
	EXPORT  disp_01_size
	EXPORT  disp_02_size            
	EXPORT  disp_03_size
	EXPORT  disp_04_size            
	EXPORT  disp_05_size
	EXPORT  disp_10_size            
	EXPORT  disp_11_size
	EXPORT  disp_12_size            
	EXPORT  disp_13_size
	EXPORT  disp_14_size            
	EXPORT  disp_15_size            
	EXPORT  disp_21_size
	EXPORT  disp_22_size            
	EXPORT  disp_23_size
	EXPORT  disp_24_size            
	EXPORT  disp_25_size       
	EXPORT  disp_61_size
	EXPORT  disp_62_size            
	EXPORT  disp_63_size
	EXPORT  disp_64_size            
	EXPORT  disp_65_size           
	EXPORT  disp_71_size
	EXPORT  disp_72_size            
	EXPORT  disp_73_size
	EXPORT  disp_74_size            
	EXPORT  disp_75_size           
;	EXPORT  disp_81_size
;	EXPORT  disp_82_size            
;	EXPORT  disp_83_size
;	EXPORT  disp_84_size            
;	EXPORT  disp_85_size
    EXPORT  imgBackground_size
	
	END  