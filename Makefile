ts:
	gcc -o time_server time_server.c -lnsl
	./time_server 8000

tc:
	gcc -o time_client time_client.c -lnsl
	./time_client 192.168.43.150 8000
es:
	gcc -o echo_server echo_server.c
	./echo_server 8000

ec:
	gcc -o echo_client echo_client.c
	./echo_client 192.168.43.150 8000
