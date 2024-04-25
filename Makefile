ncurses:
	gcc -o main main.c descend.c -lncurses -Wall -Wextra -fsanitize=undefined -fsanitize=address
51:
	sdcc -c main.c -mmcs51 --model-small
	sdcc -c descend.c -mmcs51 --model-small
	sdcc -c lcd.c -mmcs51 --model-small
	sdcc {main,descend,lcd}.rel --model-small
clean:
	rm -f main {main,descend,lcd}.{asm,lst,rel,sym,rst} main.{ihx,lk,map,mem}
flash:
	stcgal -P stc89a -b 9600 -o cpu_6t_enabled=true main.ihx
