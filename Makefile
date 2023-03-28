ts:
	gcc -o time_server time_server.c -lnsl
	./time_server 8005

tc:
	gcc -o time_client time_client.c -lnsl
	./time_client 192.168.43.150 8005
