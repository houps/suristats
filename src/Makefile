suristats.exe: main.o counter.o dbmgr.o run.o thread.o
	gcc main.o counter.o dbmgr.o run.o thread.o -Wall -o suristats.exe -l sqlite3

main.o: main.c
	gcc main.c -Wall -Wextra -c -o main.o
dbmgr.o: dbmgr.c
	gcc dbmgr.c -Wall -Wextra -c -o dbmgr.o
counter.o: counter.c
	gcc counter.c -Wall -Wextra -c -o counter.o
run.o: run.c
	gcc run.c -Wall -Wextra -c -o run.o
thread.o: thread.c
	gcc thread.c -Wall -Wextra -c -o thread.o

clean:
	rm *.exe *.o

