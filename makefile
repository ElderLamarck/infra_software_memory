# -*- MakeFile -*-

vm: elgr.c
	gcc $< -o $@
	
test1:
	./vm addresses.txt fifo fifo

test2:
	./vm addresses.txt lru fifo

test3:
	./vm addresses.txt fifo lru

test4:
	./vm addresses.txt lru lru

clean:
	rm vm