CXXFLAGS+= -g

all:  example trivial
	./example


example: example.o CoroutineStream.o
	g++ -o example example.o CoroutineStream.o

trivial: trivial.o CoroutineStream.o
	g++ -o trivial trivial.o CoroutineStream.o



clean:
	rm -f *~  *.o example trivial