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

    class ByteBuffer {
    public:

        //        ByteBuffer(const size_t& size);
        //        ByteBuffer(byte* src, const size_t& length);
        ByteBuffer(const ByteBuffer& orig);
        virtual ~ByteBuffer();

        void freeMemery(void);
        void flip(void);
        void clear(void);
        bool hasRemaining(void) const;
        void put(const uint8_t& src);
        void put(const uint16_t& src);
        void put(const uint32_t& src);
        void put(const char* src);
        void put(const std::string& src);
        void put(const uint8_t* src, const int& offset, const int& length);
        const uint8_t& get(const size_t& index) const;
        const uint8_t& get(void);
        size_t remaining(void) const;
        const size_t& position(void) const;
        size_t position(const size_t& newPosition);

        void movePosition(const size_t& length, const bool isReverse = false);
        void modifyMemery(const uint8_t* src, const size_t& offset, const size_t& length, const size_t& position);

        /**
         * If you get a ByteBuffer by this function, 
         * you need to call ByteBuffer::freeMemery() before delete it.
         */
        static ByteBuffer* allocate(const size_t& capacity);
        static ByteBuffer* wrap(uint8_t* array, const size_t& offset, const size_t& length, const size_t& capacity);
        static ByteBuffer* wrap(uint8_t* array, const size_t& capacity);

//        const uint8_t& at(const size_t& position) const;

        ByteBuffer* copy(void) const;

        const size_t& capacity(void) const {
            return _capacity;
        }

        uint8_t* array(void) const;
        std::string getString() const;
        std::string getString(const size_t& offset, const size_t& length) const;

        void outputAsHex(std::ostream& out) const;
        void outputAsHex(std::ostream& out, const size_t& offset, const size_t& length) const;
        void outputAsDec(std::ostream& out) const;
        void outputAsDec(std::ostream& out, const size_t& offset, const size_t& length) const;
//        uint8_t* arrayOfPosition() const;

    private:
        uint8_t* _hb;
        size_t _position;
        size_t _limit;
        size_t _capacity;
        size_t _mark;
        uint8_t errorCode;

        ByteBuffer(const size_t& pos, const size_t& lim, const size_t& cap, uint8_t* hb);
        static void checkBounds(const int& off, const int& len, const int& size);

    };
}
#endif /* BYTEBUFFER_H */

