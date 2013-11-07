suristats.exe: main.o counter.o dbmgr.o run.o
	gcc main.o counter.o dbmgr.o run.o -Wall -o suristats.exe -l sqlite3

main.o: main.c
	gcc main.c -Wall -Wextra -c -o main.o
dbmgr.o: dbmgr.c
	gcc dbmgr.c -Wall -Wextra -c -o dbmgr.o
counter.o: counter.c
	gcc counter.c -Wall -Wextra -c -o counter.o
run.o: run.c
	gcc run.c -Wall -Wextra -c -o run.o

clean:
	rm *.exe *.o

