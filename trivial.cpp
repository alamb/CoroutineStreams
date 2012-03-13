#include <iostream>
#include <sstream>

#include "CoroutineStream.h"

using namespace std;


//////////////////////////////
// Simplest possible example showing how to use a CoroutineStream
//////////////////////////////


struct ExampleStreamProducer : public StreamProducer 
{
    virtual void produce(CoroutineStream &str)
    {
        str.write("The producer is producing this data\n");
        str.write("The producer is also producing this data\n");
    }
};


int main() 
{
    ExampleStreamProducer p;
    CoroutineStream s(p);

    size_t cnt = 0;
    while (const StreamChunk *c = s.peekChunk()) 
    {
        cout << c->str() << endl;
        s.consumeChunk();
    }

    return 0;
}
