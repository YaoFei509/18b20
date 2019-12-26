#
#  温度探头
#
#  姚飞
#

#for Fedora use 
PREFIX=sdcc-

#PREFIX=

#
CC=$(PREFIX)sdcc
PACKIHX=$(PREFIX)packihx

# for STC11F04E
STC11FLAGS = -DSTC11F04E=1

# for STC15F104E or STC15F104W
STC15FLAGS =  -DSTC15F104=1 --iram-size 128 --no-xinit-opt 

#for STC15W204S
STC15W204SFLAGS = -DSTC15W204S=1 

all: temp_prob.ihx  temp_prob_11f04e.ihx  temp_prob_15w204s.ihx temp_prob_15f104.ihx

temp_prob.ihx:	temp_prob.rel uart.rel ds18b20.rel ds18b20_search.rel
	$(CC) $^
	$(PACKIHX) $@ > temp_prob.hex

temp_prob_11f04e.ihx: temp_prob_11.rel uart_11.rel ds18b20_1t_11.rel  ds18b20_search.rel
	$(CC) $(STC11FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_11f04e.hex

temp_prob_15w204s.ihx: temp_prob_15w204s.rel uart_15w204s.rel ds18b20_1t.rel ds18b20_search.rel
	$(CC) $(STC15W204SFLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_15w204s.hex

temp_prob_15f104.ihx: temp_prob_15f104.rel uart_15f104.rel ds18b20_1t.rel ds18b20_search.rel
	$(CC) $(STC15FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_15f104.hex

# STC15F104 only work at 1200bps
downld: temp_prob_15f104.ihx
	stcgal -b 1200 $<

#temp_prob
temp_prob.rel: temp_prob.c
	$(CC) -c $<

temp_prob_11.rel: temp_prob.c
	$(CC) $(STC11FLAGS) -o $@ -c $<

temp_prob_15w204s.rel: temp_prob.c
	$(CC) $(STC15W204SFLAGS) -o $@ -c $<

temp_prob_15f104.rel: temp_prob.c
	$(CC) $(STC15FLAGS) -o $@ -c $<

#UART
uart.rel: uart.c
	$(CC) -c $<

uart_11.rel: uart.c
	$(CC) $(STC11FLAGS) -o $@ -c $<

uart_15w204s.rel: uart.c
	$(CC) $(STC15W204SFLAGS) -o $@ -c $<

uart_15f104.rel: uart.c  
	$(CC) $(STC15FLAGS) -o $@ -c $<

#DS18B20
ds18b20.rel: ds18b20.c
	$(CC) -c $<

ds18b20_1t.rel: ds18b20_1t.c
	$(CC) $(STC15FLAGS) -c $<

ds18b20_1t_11.rel: ds18b20_1t.c
	$(CC) -o $@ $(STC11FLAGS) -c $<


#ds18B20 Search
ds18b20_search_11.rel: ds18b20_search.c
	$(CC) -o $@ $(STC11FLAGS) -c $<

ds18b20_search.rel: ds18b20_search.c
	$(CC) -c $<

log:
	git pull
	git log --format=short --graph > ChangeLog

clean:
	-rm -f *.ihx *.hex *.asm *.lnk *.lst *.map *.mem *.rel *.o  *.sym *.rst *~ *.lk *.bin

