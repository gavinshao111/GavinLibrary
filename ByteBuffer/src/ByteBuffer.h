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
     * 否则即分配空间创建新数组(allocate)，则复制构造函数不创建新数组并复制数组元素；析构函数释放数组元素。
     */
    class ByteBuffer {
    public:

        ByteBuffer(const size_t& capacity);
        ByteBuffer(uint8_t* src, const size_t& offset, const size_t& length);
        ByteBuffer(uint8_t* src, const size_t& length);

        ByteBuffer(const ByteBuffer& orig);
        virtual ~ByteBuffer();

        void flip(void);
        void clear(void);
        bool hasRemaining(void) const;
        void put(const uint8_t& src);
        void put(const uint16_t& src);
        void put(const uint32_t& src);
        void put(const char* src);
        void put(const std::string& src);
        void put(const uint8_t* src, const size_t& offset, const size_t& length);
        void put(const ByteBuffer& src, const size_t& offset, const size_t& length);
        void put(const ByteBuffer& src);

        const uint8_t& get(const size_t& index) const;

        /**
         * Reads the byte at this buffer's current position, and then increments the position.
         *
         * @return  The byte at the buffer's current position
         */
        const uint8_t& get(void);

        /**
         *
         * <p> Reads the next two bytes at this buffer's current position,
         * composing them into a short value according to the current byte order,
         * and then increments the position by two.  </p>
         *
         * @return  The short value at the buffer's current position
         */
        const uint16_t& getShort();

        size_t remaining(void) const;
        const size_t& position(void) const;
        void position(const size_t& newPosition);

        void movePosition(const size_t& length, const bool isReverse = false);
//        void modifyMemery(const uint8_t* src, const size_t& offset, const size_t& length, const size_t& position);


        //        const uint8_t& at(const size_t& position) const;

        //        ByteBuffer* copy(void) const;

        const size_t& capacity(void) const {
            return m_capacity;
        }

        uint8_t* array(void) const;
        std::string getString() const;
        std::string getString(const size_t& offset, const size_t& length) const;

        void outputAsHex(std::ostream& out) const;
        void outputAsHex(std::ostream& out, const size_t& offset, const size_t& length) const;
        void outputAsDec(std::ostream& out) const;
        void outputAsDec(std::ostream& out, const size_t& offset, const size_t& length) const;
        void readOnly(const bool& readOnly);
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
        void checkRemaining(const size_t& bufferOffset, const size_t& length) const;

    };
}
#endif /* BYTEBUFFER_H */

