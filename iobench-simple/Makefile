all: testread computebench matrixpara

testread: testread.cpp
	g++ -c  -fpermissive thpool.cpp 
	g++ -std=c++11 -g  -fpermissive testread.cpp thpool.o -o testread -lpthread 
computebench: computebench.cpp
	g++ -c  -fpermissive thpool.cpp 
	g++ -std=c++11  -fpermissive computebench.cpp thpool.o -o computebench -lpthread 

matrixpara: matrixpara.cpp
	g++ -c  -fpermissive thpool.cpp
	g++ -std=c++11  -fpermissive matrixpara.cpp thpool.o -o matrixpara -lpthread

clean:
	rm -rf testread
	rm -rf computebench	
	rm -rf matrixpara
	rm -rf *.o
