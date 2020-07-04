;File: fig0616.pep
;Computer Systems, Fifth edition
;Figure 6.16
;
         BR      main        
n1:      .BLOCK  2           ;#2d
n2:      .BLOCK  2           ;#2d
n3:      .BLOCK  2           ;#2d
;
main:    @DECI   n2,d        
         @DECI   n3,d        
         LDWA    n2,d        
         CPWA    n3,d        
         BRLT    L1          
         @DECI   n1,d        
         LDWA    n1,d        
         CPWA    n3,d        
         BRLT    L7          
         BR      L6          
         STWA    n3,d        
L1:      @DECI   n1,d        
         LDWA    n2,d        
         CPWA    n1,d        
         BRLT    L5          
         @DECO   n1,d        
         @DECO   n2,d        
L2:      @DECO   n3,d        
         RET                
L3:      @DECO   n2,d        
         @DECO   n3,d        
         BR      L9          
L4:      @DECO   n1,d        
         @DECO   n2,d        
         RET                
         STWA    n1,d        
L5:      LDWA    n3,d        
         CPWA    n1,d        
         BRLT    L3          
         @DECO   n2,d        
         @DECO   n1,d        
         BR      L2          
L6:      @DECO   n3,d        
         LDWA    n1,d        
         CPWA    n2,d        
         BRLT    L4          
         BR      L8          
L7:      @DECO   n1,d        
         @DECO   n3,d        
         @DECO   n2,d        
         RET                
L8:      @DECO   n2,d        
L9:      @DECO   n1,d        
         RET                
         .END                  
