#pragma once

#include <cstring>
#include <algorithm>
#include <memory>
#include <Poco/Exception.h>
#include <IO/BufferBase.h>


namespace IO
{

/** A simple abstract class for buffered data reading (char sequences) from somewhere.
  * Unlike std::istream, it provides access to the internal buffer,
  *  and also allows you to manually manage the position inside the buffer.
  *
  * Note! `char *`, not `const char *` is used
  *  (so that you can take out the common code into BufferBase, and also so that you can fill the buffer in with new data).
  * This causes inconveniences - for example, when using ReadBuffer to read from a chunk of memory const char *,
  *  you have to use const_cast.
  *
  * successors must implement the nextImpl() method.
  */
class ReadBuffer : public BufferBase
{
public:
    /**
	 * 创建一个buffer,并且将可读数据大小设置为0,
	 * 这样首次尝试时,next被调用用于load新数据的一部分到buffer
	 * Creates a buffer and sets a piece of available data to read to zero size,
      *  so that the next() function is called to load the new data portion into the buffer at the first try.
      */
    ReadBuffer(Position ptr, size_t size) : BufferBase(ptr, size, 0) {
        working_buffer.resize(0);
    }

    /** 
	 * 当完整数据可读时,使用该构造函数
	 * Used when the buffer is already full of data that can be read.
      *  (in this case, pass 0 as an offset)
      */
    ReadBuffer(Position ptr, size_t size, size_t offset) : BufferBase(ptr, size, offset) {}

    //和第一个构造函数一样
    void set(Position ptr, size_t size) {
        BufferBase::set(ptr, size, 0);
        working_buffer.resize(0);
    }

    /** 
	 *  读取下一个数据并且填充buffer,将位置放置在起始
	 * read next data and fill a buffer with it; set position to the beginning;
      * return `false` in case of end, `true` otherwise; throw an exception, if something is wrong
      */
    bool next()
    {
		//更新读取的字节数
        bytes += offset();
		//如果读到结尾了,那么返回false
        bool res = nextImpl();
        if (!res)
            working_buffer.resize(0);
        pos = working_buffer.begin() + working_buffer_offset;
        working_buffer_offset = 0;
        return res;
    }


    inline void nextIfAtEnd()
    {
        if (!hasPendingData())
            next();
    }

    virtual ~ReadBuffer() {}


    /** Unlike std::istream, it returns true if all data was read
      *  (and not in case there was an attempt to read after the end).
      * If at the moment the position is at the end of the buffer, it calls the next() method.
      * That is, it has a side effect - if the buffer is over, then it updates it and set the position to the beginning.
      *
      * Try to read after the end should throw an exception.
      */
    bool __attribute__((__always_inline__))  eof()
    {
        return !hasPendingData() && !next();
    }

    //跳过一个字节
    void ignore()
    {
        if (!eof())
            ++pos;
        else
            throw Poco::Exception("Attempt to read after EOF");
    }

    //跳过至多n个字节
    void ignore(size_t n)
    {
        while (n != 0 && !eof())
        {
            size_t bytes_to_ignore = std::min(static_cast<size_t>(working_buffer.end() - pos), n);
            pos += bytes_to_ignore;
            n -= bytes_to_ignore;
        }
		//如果n>0,那么就意味着已经读到eof了,不允许再继续读了
        if (n)
            throw Poco::Exception("Attempt to read after EOF");
    }

    /// 尝试跳过n个字节,不抛出异常,返回实际忽略的字节数
    size_t tryIgnore(size_t n)
    {
        size_t bytes_ignored = 0;

        while (bytes_ignored < n && !eof())
        {
            size_t bytes_to_ignore = std::min(static_cast<size_t>(working_buffer.end() - pos), n - bytes_ignored);
            pos += bytes_to_ignore;
            bytes_ignored += bytes_to_ignore;
        }

        return bytes_ignored;
    }

    /** Reads as many as there are, no more than n bytes. */
    size_t read(char * to, size_t n)
    {
        size_t bytes_copied = 0;
		//开始时,working_buffer为空,会调用next首次读取
        while (bytes_copied < n && !eof())
        {
            size_t bytes_to_copy = std::min(static_cast<size_t>(working_buffer.end() - pos), n - bytes_copied);
            ::memcpy(to + bytes_copied, pos, bytes_to_copy);
            pos += bytes_to_copy;
            bytes_copied += bytes_to_copy;
        }

        return bytes_copied;
    }

    /** Reads n bytes, if there are less - throws an exception. */
    void readStrict(char * to, size_t n)
    {
        if (n != read(to, n))
            throw Poco::Exception("Cannot read all data");
    }

    /** A method that can be more efficiently implemented in successors, in the case of reading large enough blocks.
      * The implementation can read data directly into `to`, without superfluous copying, if in `to` there is enough space for work.
      * For example, a CompressedReadBuffer can decompress the data directly into `to`, if the entire decompressed block fits there.
      * By default - the same as read.
      * Don't use for small reads.
      */
    virtual size_t readBig(char * to, size_t n)
    {
        return read(to, n);
    }

protected:
    /// The number of bytes to ignore from the initial position of `working_buffer` buffer.
    size_t working_buffer_offset = 0;

private:
    /** Read the next data and fill a buffer with it.
      * Return `false` in case of the end, `true` otherwise.
      * Throw an exception if something is wrong.
      */
    virtual bool nextImpl() {
        return false;
    };
};


using ReadBufferPtr = std::shared_ptr<ReadBuffer>;


}
