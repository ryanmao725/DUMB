all:
	@gcc DUMBclient.c -o DUMBclient
	@gcc DUMBserver.c -o DUMBserve -lpthread

client:
	@gcc DUMBclient.c -o DUMBclient

serve:
	@gcc DUMBserver.c -o DUMBserve -lpthread

clean:
	@rm ./DUMBserve ./DUMBclient
