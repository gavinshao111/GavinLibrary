/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ByteBuffer.cpp
 * Author: 10256
 * 
 * Created on 2017年1月19日, 下午5:12
 */

#include "ByteBuffer.h"
#include <string.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace bytebuf;
using namespace std;


// public

ByteBuffer::ByteBuffer(const size_t& capacity) {
    new (this)ByteBuffer(0, capacity, capacity, new uint8_t[capacity], false);
}
// public

ByteBuffer::ByteBuffer(uint8_t* src, const size_t& offset, const size_t& length) {
    new (this)ByteBuffer(0, length, length, src + offset, true);
}
// public

ByteBuffer::ByteBuffer(uint8_t* src, const size_t& length) {
    new (this)ByteBuffer(src, 0, length);
}
// private

ByteBuffer::ByteBuffer(const size_t& pos, const size_t& lim, const size_t& cap, uint8_t* hb, const bool& isWrapped) :
m_hb(hb), m_position(pos), m_limit(lim), m_capacity(cap), m_isWarpped(isWrapped), m_readOnly(false) {
    if (cap == 0)
        throw ByteBufferException("ByteBuffer::ByteBuffer(): IllegalArgument, cap == 0");
    if (hb == NULL)
        throw ByteBufferException("ByteBuffer::ByteBuffer(): IllegalArgument, hb == NULL");
}

ByteBuffer::ByteBuffer(const ByteBuffer& orig) :
m_position(orig.m_position),
m_limit(orig.m_limit),
m_capacity(orig.m_capacity),
m_isWarpped(orig.m_isWarpped),
m_readOnly(orig.m_readOnly) {
    if (m_isWarpped)
        m_hb = orig.m_hb;
    else {
        m_hb = new uint8_t[m_capacity];
        memcpy(m_hb, orig.m_hb, m_capacity);
    }
    // test
    cout << "copy construct" << endl;
}

ByteBuffer::~ByteBuffer() {
    if (!m_isWarpped)
        delete m_hb;
}
//ByteBuffer* ByteBuffer::allocate(const size_t& capacity) {
//    return new ByteBuffer(0, capacity, capacity, new uint8_t[capacity]);
//}
//
//void ByteBuffer::freeMemery() {
//    if (_hb) {
//        delete _hb;
//        _hb = NULL;
//    }
//}
//
//ByteBuffer* ByteBuffer::wrap(uint8_t* array, const size_t& offset, const size_t& length, const size_t& capacity) {
//    //checkBounds(offset, length, sizeof(array));
//    try {
//        return new ByteBuffer(offset, offset + length, capacity, array);
//    } catch (exception &e) {
//        throw ByteBufferException("ByteBuffer::wrap(): IndexOutOfBounds");
//    }
//}
//
//ByteBuffer* ByteBuffer::wrap(uint8_t* array, const size_t& capacity) {
//    return wrap(array, 0, capacity, capacity);
//}

void ByteBuffer::flip() {
    m_limit = m_position;
    m_position = 0;
}

void ByteBuffer::clear() {
    m_position = 0;
    m_limit = m_capacity;
}

void ByteBuffer::rewind() {
    m_position = 0;
}

bool ByteBuffer::hasRemaining() const {
    return m_position < m_limit;
}

//void ByteBuffer::checkBounds(const size_t& off, const size_t& len, const size_t& cap) {
//    if ((off | len | (off + len) | (cap - (off + len))) < 0)
//        throw ByteBufferException("ByteBuffer::checkBounds(): IndexOutOfBounds");
//}

void ByteBuffer::checkRemaining(const size_t& bufferOffset, const size_t& length) const {
    if (m_position + bufferOffset + length > m_limit) {
        stringstream s;
        s << "ByteBuffer::checkRemaining(): BufferOverflow"
                << ", m_position: " << m_position
                << ", bufferOffset: " << bufferOffset
                << ", length: " << length
                << ", m_limit: " << m_limit;
        throw ByteBufferException(s.str());
    }
}

size_t ByteBuffer::remaining() const {
    return m_limit - m_position;
}

void ByteBuffer::put(const uint8_t* src, const size_t& offset, const size_t& length) {
    size_t end = offset + length;
    for (size_t i = offset; i < end; i++)
        put(src[i]);
}

void ByteBuffer::put(const uint8_t& src) {
    if (m_readOnly)
        throw ByteBufferException("ByteBuffer::put(): read only");

    checkRemaining(0, 1);
    m_hb[m_position++] = src;
}

void ByteBuffer::put(const uint16_t& src) {
    put((uint8_t*) & src, 0, 2);
}

void ByteBuffer::put(const uint32_t& src) {
    put((uint8_t*) & src, 0, 4);
}

void ByteBuffer::put(const string& src) {
    put((uint8_t*) src.data(), 0, src.length());
}

void ByteBuffer::put(const char* src) {
    put((uint8_t*) src, 0, strlen(src));
}

void ByteBuffer::put(const ByteBuffer& src, const size_t& offset, const size_t& length) {
    if (src.remaining() < offset + length) {
        stringstream s;
        s << "ByteBuffer::put(): IllegalArgument"
                << ", remaining: " << src.remaining()
                << ", offset: " << offset
                << ", length: " << length;
        throw ByteBufferException(s.str());
    }
    put(src.array(), src.position() + offset, length);
}

void ByteBuffer::put(ByteBuffer& src, const size_t& length) {
    if (&src == this)
        throw ByteBufferException("ByteBuffer::put(): IllegalArgument, &src == this");

    if (length > src.remaining()) {
        stringstream s;
        s << "ByteBuffer::put(): IllegalArgument"
                << ", remaining: " << src.remaining()
                << ", length: " << length;
        throw ByteBufferException(s.str());
    }
    for (size_t i = 0; i < length; i++)
        put(src.get());
}

void ByteBuffer::put(ByteBuffer& src) {
    put(src, src.remaining());
}

const uint8_t& ByteBuffer::get(const size_t& index) const {
    if (index > m_capacity) {
        stringstream s;
        s << "ByteBuffer::get(): IllegalArgument"
                << ", index: " << index
                << ", m_capacity: " << m_capacity;
        throw ByteBufferException(s.str());
    }
    return m_hb[index];
}

const uint8_t& ByteBuffer::get() {
    return get(m_position++);
}

const uint16_t& ByteBuffer::getShort() {
    checkRemaining(0, 2);
    m_position += 2;
    return *(uint16_t*) (m_hb + m_position - 2);
}

const size_t& ByteBuffer::position() const {
    return m_position;
}

void ByteBuffer::position(const size_t& newPosition) {
    if ((newPosition > m_limit)) {
        stringstream s;
        s << "ByteBuffer::position(): IllegalArgument"
                << ", newPosition: " << newPosition
                << ", m_limit: " << m_limit;
        throw ByteBufferException(s.str());
    }

    m_position = newPosition;
}

const size_t& ByteBuffer::limit() const {
    return m_limit;
}

void ByteBuffer::limit(const size_t& newLimit) {
    if ((newLimit > m_capacity)) {
        stringstream s;
        s << "ByteBuffer::limit(): IllegalArgument"
                << ", newLimit: " << newLimit
                << ", m_capacity: " << m_capacity;
        throw ByteBufferException(s.str());
    }

    m_limit = newLimit;
    if (m_position > m_limit) m_position = m_limit;
}

void ByteBuffer::movePosition(const size_t& length, const bool isReverse/* = false*/) {
    size_t newPos = isReverse ? (m_position - length) : (m_position + length);
    if ((newPos > m_limit)) {
        stringstream s;
        s << "ByteBuffer::movePosition(): IllegalArgument"
                << ", newPos: " << newPos
                << ", m_limit: " << m_limit;
        throw ByteBufferException(s.str());
    }

    m_position = newPos;
}

//void ByteBuffer::modifyMemery(const uint8_t* src, const size_t& offset, const size_t& length, const size_t& position) {
//    if (m_readOnly)
//        throw ByteBufferException("ByteBuffer::put(): read only");
//    if (m_position + length > m_limit)
//        throw ByteBufferException("ByteBuffer::modifyMemery(): IllegalArgument");
//    for (size_t i = 0; i < length; i++)
//        m_hb[i + position] = src[offset + i];
//}

//const uint8_t& ByteBuffer::at(const size_t& position) const {
//    if (position > _limit)
//        throw ByteBufferException("ByteBuffer::at(): IllegalArgument");
//    return _hb[position];
//}

//ByteBuffer* ByteBuffer::copy() const {
//    if (0 >= _capacity || NULL == _hb)
//        return NULL;
//    ByteBuffer* copy = new ByteBuffer(_position, _limit, _capacity, new uint8_t[_capacity]);
//    memcpy(_hb, copy->_hb, _capacity);
//    return copy;
//}

uint8_t* ByteBuffer::array() const {
    if (m_hb == NULL)
        throw ByteBufferException("ByteBuffer::array(): m_hb == NULL");

    return m_hb;
}

string ByteBuffer::toString() const {
    return toString(0, remaining());
}

string ByteBuffer::toString(const size_t& offset, const size_t& length) const {
    checkRemaining(offset, length);
    string str((char*) &m_hb[m_position + offset], length);
    return str;
}

void ByteBuffer::outputAsHex(ostream& out) const {
    outputAsHex(out, 0, remaining());
}

void ByteBuffer::outputAsHex(ostream& out, const size_t& offset, const size_t& length) const {
    checkRemaining(offset, length);
    for (size_t i = 0; i < length; i++)
        out << hex << setw(3) << setfill(' ') << (short) m_hb[m_position + offset + i];
}

void ByteBuffer::outputAsDec(ostream& out) const {
    outputAsDec(out, m_position, remaining());
}

void ByteBuffer::outputAsDec(ostream& out, const size_t& offset, const size_t& length) const {
    checkRemaining(offset, length);
    for (size_t i = 0; i < length; i++)
        out << setw(4) << setfill(' ') << (short) m_hb[m_position + offset + i];
}

void ByteBuffer::outputAsOct(ostream& out) const {
    outputAsOct(out, m_position, remaining());
}

void ByteBuffer::outputAsOct(ostream& out, const size_t& offset, const size_t& length) const {
    checkRemaining(offset, length);
    for (size_t i = 0; i < length; i++)
        //        out << (short) _hb[_position + offset + i] << ' ';
        out << setw(4) << setfill(' ') << oct << (short) m_hb[m_position + offset + i];
}

void ByteBuffer::readOnly(const bool& readOnly) {
    m_readOnly = readOnly;
}
