loop:   
	ADD $1, $2, $3      
        ANDI $4, $1,15     
        SUB $5, $4, $3      
        BEQ $5, $0, end     
        ADDI $2, $2,1                 
end:    
	ADD $6, $1, $0      