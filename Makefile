objects = ./src/utility.cpp ./src/foperator.cpp ./src/ubuff.cpp ./src/uconn.cpp
all:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp
test1:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
test2:
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp
clean:
	rm src/*.o 
	rm src/*.a