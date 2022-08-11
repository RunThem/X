SHELL = /bin/bash
CC = clang -m64

who: who.c utmplib.c
	$(CC) who.c utmplib.c -o who
	./who

cp: cp.c
	$(CC) cp.c -o cp

buffer_test: buffer_test.c
	$(CC) buffer_test.c -o buffer_test
	dd if=/dev/random of=test.file bs=1M count=10
	@for i in 1 4 16 128 256 512 1024 2048 4096 8192 16384;\
	do\
		echo -n "++++++++++++++++++++++++++++++";\
		echo -e "\nbuffer size $${i} byte.";\
		time ./buffer_test $${i};\
		echo "++++++++++++++++++++++++++++++";\
	done

2_6: 2_6.c
	$(CC) 2_6.c -o 2_6
	cp Makefile test
	./2_6
	cat test

2_9: 2_9.c
	$(CC) 2_9.c -o 2_9
	touch test_2_9
	./2_9 test_2_9

whoami: whoami.c
	$(CC) whoami.c -o whoami
	./whoami

ac: ac.c
	$(CC) ac.c -o ac

last: last.c
	$(CC) last.c -o last

cat: cat.c
	$(CC) cat.c -o cat

head: head.c
	$(CC) head.c -o head

tail: tail.c
	$(CC) tail.c -o tail

clean:
	$(RM) who cp buffer_test test.file 2_6 test 2_9 test_2_9 whoami ac last cat head tail

PYTHON: clean
