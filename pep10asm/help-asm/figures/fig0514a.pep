;File: fig0514a.pep
;Computer Systems, Fifth edition
;Figure 5.14(a)
;
         LDBA    0x0013,d    
         STBA    0xFAAC,d    
         LDBA    0x0014,d    
         STBA    0xFAAC,d    
         LDBA    0x0015,d    
         STBA    0xFAAC,d    
         RET                
         .ASCII  "Pun"       
         .END                  
