
CC=sdcc-sdcc -mmcs51

all: temp_prob.ihx  temp_prob_stc15f.ihx

temp_prob.ihx:	temp_prob.c
	$(CC) $<

downld: temp_prob.ihx
	stcisp -f $<

temp_prob_stc15f.ihx: temp_prob_stc15f.rel soft_uart.rel ds18b20_1t.rel
	$(CC) -o $@ $^

temp_prob_stc15f.rel: temp_prob_stc15f.c
	$(CC) -c $<

soft_uart.rel: soft_uart.c  
	$(CC) -c $<

ds18b20_1t.rel: ds18b20_1t.c
	$(CC) -c $<

clean:
	-rm -f temp_prob.ihx *.asm *.lnk *.lst *.map *.mem *.rel *.o  *.sym *.rst *~ *.lk *.bin

