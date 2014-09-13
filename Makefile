#
#
#  温度探头
#
#  姚飞
#

PREFIX=
CC=$(PREFIX)sdcc
PACKIHX=$(PREFIX)packihx

# for STC15F104E
STC15FLAGS =  --iram-size 128 --no-xinit-opt 

all: temp_prob.ihx  temp_prob_stc15f.ihx

temp_prob.ihx:	temp_prob.c
	$(CC) $<
	$(PACKIHX) $@ > temp_prob.hex

downld: temp_prob.ihx
	stcisp -f $<

temp_prob_stc15f.ihx: temp_prob_stc15f.rel soft_uart.rel ds18b20_1t.rel
	$(CC) $(STC15FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_stc15f.hex

temp_prob_stc15f.rel: temp_prob_stc15f.c
	$(CC) $(STC15FLAGS) -c $<

soft_uart.rel: soft_uart.c  
	$(CC) $(STC15FLAGS) -c $<

ds18b20_1t.rel: ds18b20_1t.c
	$(CC) $(STC15FLAGS) -c $<

clean:
	-rm -f *.ihx *.hex *.asm *.lnk *.lst *.map *.mem *.rel *.o  *.sym *.rst *~ *.lk *.bin

