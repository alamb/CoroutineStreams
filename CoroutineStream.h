#ifndef COROUTINE_STREAM_H
#define COROUTINE_STREAM_H

#include <ucontext.h>
#include <list>
#include <string>

#define CR_STREAM_STACKSIZE 80000
#define CR_STREAM_BUFFERSIZE 10


struct StreamChunk {
    StreamChunk() : sz(0) {}
    size_t  size()    const { return sz; }
    size_t  avail()   const { return sizeof(buf)  - sz; }
    std::string str() const { return std::string(buf, sz); }
    
    char buf[CR_STREAM_BUFFERSIZE];
    size_t sz;
};


struct BaseStream
{

    // write sz bytes from data into the stream;
    virtual void write(const char *data, size_t sz);

    // Returns next chunk to read, or null of no such chunk
    virtual const StreamChunk *peekChunk();

    // Consumes the current chunk (or no op if stream is empty);
    void consumeChunk();

    // Do we have any data in the stream
    bool empty() const { return chunks.empty(); }

    // How man chunks do we have?
    size_t size() const { return chunks.size(); }

private:
    std::list<StreamChunk> chunks;
};


///////////////////////////
// Interface for an incremental stream
//
// A producer function is invoked which writes data into what looks like an
// infinte stream but really is a coroutine that swaps back to consumer at
// various points.
///////////////////////////

struct CoroutineStream;

// producer interface
struct StreamProducer
{
    virtual void produce(CoroutineStream &str) = 0;
};

struct CoroutineStream : public BaseStream 
{
    CoroutineStream(StreamProducer &prod);

    // write sz bytes from data into the stream;
    virtual void write(const char *data, size_t sz);

    // Convenience: write a std::string
    void write (const std::string &str) { write(str.c_str(), str.size()); }

    // Returns next chunk to read, or null of no such chunk
    virtual const StreamChunk *peekChunk();

private:
    static void producerShim(int a0, int a1);

    StreamProducer &_producer;
    bool _producerDone;
    bool _producerStarted;

    char _pstack[CR_STREAM_STACKSIZE];
    ucontext_t consumer_context, producer_context;
};




#endif /* COROUTINE_STREAM_H */
