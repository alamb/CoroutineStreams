#include "CoroutineStream.h"
#include <string.h>


void BaseStream::write(const char *data, size_t sz)
{
    const char *p = data;
    size_t remain = sz;

    // while we have data remaining...
    while (remain > 0) {
        // allocate new chunk if necessary
        if (chunks.empty() || chunks.back().avail() == 0) {
            chunks.push_back(StreamChunk());
        }
        
        // Copy
        StreamChunk &c = chunks.back();
        size_t cpySz =  c.avail() < remain ? c.avail() : remain;
        char *writep = c.buf+c.sz;
        memcpy(writep, p, cpySz);
        

        // update our pointers and remaining counts
        c.sz   += cpySz;
        remain -= cpySz;
        p      += cpySz;
    }
}

const StreamChunk *BaseStream::peekChunk()
{
    return (chunks.empty()) ? NULL : &chunks.front();
}

// Consumes the current chunk (or no op if stream is empty);
void BaseStream::consumeChunk()
{
    if (!chunks.empty()) chunks.pop_front(); 
}



// Can only pass integer args to makecontext. This code wrapes a pointer
// into two integer args in a very non portable way.
struct ContextArgHacker {
    ContextArgHacker(CoroutineStream *ptr) : _ptr(ptr) {}
    ContextArgHacker(int a0, int a1) { getArr()[0] = a0; getArr()[1] = a1; }

    int getA0()              { return getArr()[0]; }
    int getA1()              { return getArr()[1]; }
    int *getArr()            { return reinterpret_cast<int*>(&_ptr); }
    CoroutineStream* getPtr() { return _ptr; }

private:
    CoroutineStream *_ptr;
};




void CoroutineStream::producerShim(int a0, int a1) 
{
    // get the arguments back into a pointer
    ContextArgHacker hacker(a0,a1);
    CoroutineStream *_this = hacker.getPtr();

    _this->_producer.produce(*_this);
    _this->_producerDone = true;
}

CoroutineStream::CoroutineStream(StreamProducer &prod) : 
    _producer(prod), 
    _producerDone(false), 
    _producerStarted(false)
{
}


void CoroutineStream::write(const char *data, size_t sz)
{
    // This is getting called from the producer. We only write if there is no
    // more data in the stream (or maybe eventually we'll limit the size)
    if (!empty()) {
        // swap back to consumer context to consume data until more data is needed
        int stat = swapcontext(&producer_context, &consumer_context);
    }
    // now pass it along to the underlying stream
    BaseStream::write(data, sz);
}

const StreamChunk *CoroutineStream::peekChunk()
{
    // If stream is empty or we only have a single (potentially half full
    // buffer), get more by running the producer
    if (!BaseStream::peekChunk() || this->size() == 1) {
        if (!_producerStarted) {
            ContextArgHacker hacker(this);

            int stat = getcontext(&producer_context);
            producer_context.uc_stack.ss_sp   = _pstack;
            producer_context.uc_stack.ss_size = sizeof(_pstack);
            producer_context.uc_link = &consumer_context;
            makecontext(&producer_context, 
                        reinterpret_cast<void (*)()>(producerShim), 
                        2, hacker.getA0(), hacker.getA1());
            _producerStarted = true;
        } 

        // if the producer isn't done, call it to give a chance to make more
         if (!_producerDone) {
            int stat = swapcontext(&consumer_context, &producer_context);
        } 
        // producer is done, no more data is coming
    }
    

    return BaseStream::peekChunk();
}
