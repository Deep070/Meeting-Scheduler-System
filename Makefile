CFLAGS=g++ -w


# Define the targets and their dependencies
all: serverM serverA serverB client

serverM: serverM.cpp
	$(CFLAGS) -o serverM serverM.cpp

serverA: serverA.cpp
	$(CFLAGS) -o serverA serverA.cpp
	
serverB: serverB.cpp
	$(CFLAGS) -o serverB serverB.cpp

client: client.cpp
	$(CFLAGS) -o client client.cpp
	
clean:
	rm -f serverM serverA serverB client

