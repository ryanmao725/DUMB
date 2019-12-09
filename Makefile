all:
	@gcc DUMBclient.c -o DUMBclient
	@gcc DUMBserve.c -o DUMBserve

clean:
	@rm *.o
