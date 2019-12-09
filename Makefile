all:
	@gcc DUMBclient.c -o DUMBclient
	@gcc DUMBserver.c -o DUMBserve

client:
	@gcc DUMBclient.c -o DUMBclient

serve:
	@gcc DUMBserver.c -o DUMBserve

clean:
	@rm ./DUMBserve ./DUMBclient
