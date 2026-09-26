CC := gcc

process : process.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
	
file_io : file_io .c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

erlou : erlou.c
	-$(CC) -o $@ $^


execve_test: execve_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@