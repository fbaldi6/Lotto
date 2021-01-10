all: lotto_client lotto_server

#make rule per il client
lotto_client: lotto_client.o utility.o
			gcc -Wall lotto_client.o utility.o -o lotto_client

#make rule per il server
lotto_server: lotto_server.o utility.o
			gcc -Wall lotto_server.o utility.o -o lotto_server


#pulizia dei file della compilazione
clean: 
		rm *o lotto_client lotto_server
