/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ByteBuffer.h
 * Author: 10256
 *
 * Created on 2017年1月19日, 下午5:12
 */

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <stdexcept>
#include <ostream>
#include <memory>

namespace bytebuf {
    //#define IndexOutOfBounds 1
    //#define BufferOverflow 2
    //#define IllegalArgument 3
    //#define UnsupportedOperation 4

    class ByteBufferException : public std::runtime_error {
    private:
        const int errNo;
    public:

        ByteBufferException(const ByteBufferException& e) : std::runtime_error(e.what()), errNo(e.errNo) {
        }

        ByteBufferException(const std::string& reason, int vendorCode) :
        std::runtime_error(reason),
        errNo(vendorCode) {
        }

        ByteBufferException(const std::string& reason) : std::runtime_error(reason), errNo(0) {
        }

        ByteBufferException() : std::runtime_error(""), errNo(0) {
        }

        virtual ~ByteBufferException() throw () {
        };

        int getErrorCode() const {
            return errNo;
        }
    };

    /**
     * 若构造方式为包装已有数组(warp)，则复制构造函数不复制数组元素，只复制数组地址；析构函数不释放数组元素。
     * 否则即分配空间创建新数组(allocate)，复制构造函数创建新数组并复制数组元素；析构函数释放数组元素。
     */
    class ByteBuffer {
    public:

        ByteBuffer(const size_t& capacity);
        // Warpped
        ByteBuffer(uint8_t* src, const size_t& offset, const size_t& length);
        // Warpped
        ByteBuffer(uint8_t* src, const size_t& length);

        ByteBuffer(const ByteBuffer& orig);
        virtual ~ByteBuffer();

        ByteBuffer& flip();
        ByteBuffer& clear();
        ByteBuffer& rewind();
        bool hasRemaining() const;
        ByteBuffer& put(const uint8_t& src);
        ByteBuffer& put(const uint16_t& src);
        ByteBuffer& put(const uint32_t& src);
        ByteBuffer& put(const char* src);
        ByteBuffer& put(const std::string& src);
        ByteBuffer& put(const uint8_t* src, const size_t& offset, const size_t& length);

        /**
         * This method copies <i>length</i> of bytes from src.position + offset
         * from src into this buffer, starting at each buffer's current position.
         * Only this buffer's position incremented by <i>length</i>.
         * 
         * Offset + length must be smaller than src.remaining(), otherwise,
         * IllegalSource exception thrown. And length must be smaller than this 
         * buffer' remaining, otherwise BufferOverflow exception thrown.
         * 
         * @param src
         * The source buffer from which bytes are to be read
         * 
         * @param offset 
         * offset start from src.position
         * 
         * @param length
         * length of bytes to be copied from source.
         */
        ByteBuffer& put(const ByteBuffer& src, const size_t& offset, const size_t& length);

        /**
         * This method copies <i>length</i> of bytes remaining in the given 
         * source buffer into this buffer, starting at each buffer's current 
         * position. The positions of both buffers are then incremented by 
         * <i>length</i>.
         * 
         * @param src
         * The source buffer from which bytes are to be read; must not be this 
         * buffer
         * 
         * @param length
         * length of bytes to be copied from source.
         */
        ByteBuffer& put(ByteBuffer& src, const size_t& length);

        /**
         * This method copies <i>n</i> = </i>src.remaining()</i> of bytes 
         * remaining in the given source buffer into this buffer, starting at 
         * each buffer's current position. The positions of both buffers are 
         * then incremented by <i>n</i>.
         * 
         * @param src
         * The source buffer from which bytes are to be read; must not be this 
         * buffer
         */
        ByteBuffer& put(ByteBuffer& src);

        const uint8_t& get(const size_t& index) const;

        /**
         * Reads the byte at this buffer's current position, and then increments
         * the position.
         *
         * @return  The byte at the buffer's current position
         */
        const uint8_t& get();

        /**
         * Relative bulk <i>get</i> method.
         *
         * <p> This method transfers bytes from this buffer into the given
         * destination array.  If there are fewer bytes remaining in the
         * buffer than are required to satisfy the request, that is, if
         * <tt>length</tt>&nbsp;<tt>&gt;</tt>&nbsp;<tt>remaining()</tt>, then no
         * bytes are transferred and a {@link ByteBufferException} is
         * thrown.  If <tt>dst_length</tt> is smaller than <tt>length</tt>,
         * a {@link ByteBufferException} is thrown.
         * 
         * <p> Otherwise, this method copies <tt>length</tt> bytes from this
         * buffer into the given array, starting at the current position of this
         * buffer and at the given offset in the array.  The position of this
         * buffer is then incremented by <tt>length</tt>.
         *
         * <p> In other words, an invocation of this method of the form
         * <tt>src.get(dst,&nbsp;off,&nbsp;len)</tt> has exactly the same effect as
         * the loop
         *
         * <pre>{@code
         *     for (int i = off; i < off + len; i++)
         *         dst[i] = src.get():
         * }</pre>
         *
         * except that it first checks that there are sufficient bytes in
         * this buffer and it is potentially much more efficient.
         *
         * @param  dst
         *         The array into which bytes are to be written
         *
         * @param  offset
         *         The offset within the array of the first byte to be
         *         written; must be non-negative and no larger than
         *         <tt>dst.length</tt>
         *
         * @param  length
         *         The maximum number of bytes to be written to the given
         *         array; must be non-negative and no larger than
         *         <tt>dst.length - offset</tt>
         * 
         * @param  dst_length
         *         The length of dst.
         *
         * @return  This buffer
         *
         * @throws  BufferUnderflowException
         *          If there are fewer than <tt>length</tt> bytes
         *          remaining in this buffer
         *
         * @throws  IndexOutOfBoundsException
         *          If the preconditions on the <tt>offset</tt> and <tt>length</tt>
         *          parameters do not hold
         */
        void get(uint8_t* const dst, const size_t& offset, const size_t& length, const size_t& dst_length);

        /**
         *
         * <p> Reads the next two bytes at this buffer's current position,
         * composing them into a short value according to the current byte order,
         * and then increments the position by two.  </p>
         *
         * @return  The short value at the buffer's current position
         */
        const uint16_t& getShort();

        size_t remaining() const;
        const size_t& position() const;
        ByteBuffer& position(const size_t& newPosition);

        const size_t& limit() const;
        ByteBuffer& limit(const size_t& newLimit);

        ByteBuffer& movePosition(const size_t& length, const bool isReverse = false);
        //        void modifyMemery(const uint8_t* src, const size_t& offset, const size_t& length, const size_t& position);


        //        const uint8_t& at(const size_t& position) const;

        //        ByteBuffer* copy() const;

        const size_t& capacity() const {
            return m_capacity;
        }

        uint8_t* array() const;
        std::string to_str() const;
        std::string to_str(const size_t& offset, const size_t& length) const;

        std::string to_hex() const;
        std::string to_hex(const size_t& offset, const size_t& length) const;
        std::string to_dec() const;
        std::string to_dec(const size_t& offset, const size_t& length) const;
        std::string to_oct() const;
        std::string to_oct(const size_t& offset, const size_t& length) const;


        ByteBuffer& readOnly(const bool& readOnly);
        const bool& readOnly() const;
        //        uint8_t* arrayOfPosition() const;

    private:
        uint8_t* m_hb;
        size_t m_position;
        size_t m_limit;
        size_t m_capacity;
        size_t _mark;
        uint8_t errorCode;
        bool m_isWarpped;
        bool m_readOnly;

        ByteBuffer(const size_t& pos, const size_t& lim, const size_t& cap, uint8_t* hb, const bool& isWrapped);
        //        static void checkBounds(const int& off, const int& len, const int& cap);

        /**
         * check whether remaining is more than bufferOffset + length
         * @param bufferOffset
         * offset from current position
         * @param length
         */
        void checkRemaining(const size_t& bufferOffset, const size_t& length) const;

    };
    typedef std::shared_ptr<ByteBuffer> sharedptr_t;
}
#endif /* BYTEBUFFER_H */

