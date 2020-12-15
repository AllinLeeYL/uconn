objects = ./src/utility.cpp ./src/foperator.cpp ./src/ubuff.cpp ./src/uconn.cpp
all:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp
	g++ -g -o src/main.out $(objects) ./src/main.cpp
test1:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
test2:
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp

goodnet:
	tc qdisc add dev lo root netem loss 1% delay 5ms
cleargood:
	tc qdisc del dev lo root netem loss 1% delay 5ms

normalnet:
	tc qdisc add dev lo root netem loss 3% delay 30ms corrupt 0.2% reorder 2% 8%
clearnormal:
	tc qdisc del dev lo root netem loss 3% delay 30ms corrupt 0.2% reorder 2% 8%

badnet:
	tc qdisc add dev lo root netem loss 12% delay 170ms corrupt 3% reorder 5% 10%
clearbad:
	tc qdisc del dev lo root netem loss 12% delay 170ms corrupt 3% reorder 5% 10%
clean:
	rm src/*.out