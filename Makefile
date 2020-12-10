objects = ./src/utility.cpp ./src/foperator.cpp ./src/ubuff.cpp ./src/uconn.cpp
all:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp
	g++ -o src/main.out $(objects) ./src/main.cpp
test1:
	g++ -g -o src/test1.out $(objects) ./src/test1.cpp
test2:
	g++ -g -o src/test2.out $(objects) ./src/test2.cpp
loss:
	tc qdisc add dev lo root netem loss 2%
greatloss:
	tc qdisc add dev lo root netem loss 20%
delay:
	tc qdisc add dev lo root netem delay 40ms
greatdelay:
	tc qdisc add dev lo root netem delay 200ms
clearloss:
	tc qdisc del dev lo root netem loss 2%
cleargreatloss:
	tc qdisc del dev lo root netem loss 20%
cleardealy:
	tc qdisc del dev lo root netem dealy 40ms
cleargreatdelay:
	tc qdisc del dev lo root netem delay 200ms
clean:
	rm src/*.o 
	rm src/*.a