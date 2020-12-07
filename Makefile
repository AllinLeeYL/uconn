objects = ./src/utility.cpp ./src/ubuff.cpp ./src/uconn.cpp ./src/ftransfer.cpp
test1:
	g++ -g -o src/test1.out -pthread $(objects) ./src/test1.cpp
	./src/test1.out
test2:
	g++ -g -o src/test2.out -pthread $(objects) ./src/test2.cpp
	./src/test2.out
clean:
	rm src/*.o 
	rm src/*.a