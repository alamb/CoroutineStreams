#include <iostream>
#include <sstream>

#include "CoroutineStream.h"



//////////////////////////////
// Test rig for implementation
//////////////////////////////


struct ExampleStreamProducer : public StreamProducer 
{
    virtual void produce(CoroutineStream &str)
    {
        for(size_t i=0; i<2; i++) {
            std::stringstream ss;
            ss << "ExampleData " << i;

            str.write(ss.str());
            std::cout << "Producer wrote '" 
                      << ss.str() << "' (" << ss.str().size() << ")" 
                      << std::endl;
        }
    }
};


int main() 
{
    ExampleStreamProducer p;
    CoroutineStream s(p);

    size_t cnt = 0;
    while (const StreamChunk *c = s.peekChunk()) {
        std::cout << "main stream read " 
                  << c->size() 
                  << " bytes of data: " << c->str() << std::endl;
        s.consumeChunk();
    }

    return 0;
}
