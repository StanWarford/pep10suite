;File: fig0522.pep
;Computer Systems, Fifth edition
;Figure 5.22
;
         BR      main        
ch:      .BLOCK  1           ;global variable #1c
j:       .BLOCK  2           ;global variable #2d
;
main:    @CHARI  ch,d    ;scanf("%c %d", &ch, &j)
         @DECI   j,d         
         LDWA    j,d         ;j += 5
         ADDA    5,i         
         STWA    j,d         
         LDBA    ch,d        ;ch++
         ADDA    1,i         
         STBA    ch,d        
         @CHARO  ch,d        ;printf("%c\n%d\n", ch, j)
         @CHARO  '\n',i      
         @DECO   j,d         
         @CHARO  '\n',i      
         RET                
         .END                  
