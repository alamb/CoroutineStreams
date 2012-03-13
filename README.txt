*********************
INTRODUCTION
*********************

I couldn't find example Linux C++ code that implemented a stream
abstraction for when you have a producer and consumer, want to
synchronize their execution (don't let the producer get ahead of the
consumer), and minimize buffering.

One classic way to do this is with threads and mutexes and condition
variables. Another classic mechanism is coroutines
(http://en.wikipedia.org/wiki/Coroutine). 

I needed this for an internal project but figured other people might
find it interesting too.

*********************
EXAMPLE:
*********************

What is important to note in the following code (in trivial.cpp) is
that the producer gets called "on demand" by the client.

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

*********************
CAVEATS:
*********************

I am sure this code won't work on anything but 64-bit Linux. Feel free
to help port it.



