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
#include <stdio.h>
#include <iomanip>

using namespace bytebuf;
using namespace std;

//ByteBuffer::ByteBuffer(const size_t& size) : _hb(new byte[size]), _position(0), _limit(size), _capacity(size) {
//}
//
//ByteBuffer::ByteBuffer(byte* src, const size_t& length) : _hb(src), _position(0), _limit(length), _capacity(length) {
//}

ByteBuffer::ByteBuffer(const size_t& pos, const size_t& lim, const size_t& cap, uint8_t* hb) :
_hb(hb), _position(pos), _limit(lim), _capacity(cap) {
    if (cap < 0)
        throw ByteBufferException("ByteBuffer::ByteBuffer(): IllegalArgument");
}

ByteBuffer::ByteBuffer(const ByteBuffer& orig) : _hb(orig._hb), _position(orig._position), _limit(orig._limit), _capacity(orig._capacity) {
}

ByteBuffer::~ByteBuffer() {
}

void ByteBuffer::flip(void) {
    _limit = _position;
    _position = 0;
}

void ByteBuffer::clear(void) {
    _position = 0;
    _limit = _capacity;
}

bool ByteBuffer::hasRemaining() const {
    return _position < _limit;
}

void ByteBuffer::checkBounds(const int& off, const int& len, const int& size) {
    if ((off | len | (off + len) | (size - (off + len))) < 0)
        throw ByteBufferException("ByteBuffer::checkBounds(): IndexOutOfBounds");
}

size_t ByteBuffer::remaining(void) const {
    return _limit - _position;
}

void ByteBuffer::put(const uint8_t* src, const int& offset, const int& length) {
    checkBounds(offset, length, offset + length);

    if (length > remaining())
        throw ByteBufferException("ByteBuffer::put(): BufferOverflow");

    size_t end = offset + length;
    for (size_t i = offset; i < end; i++)
        put(src[i]);
}

void ByteBuffer::put(const uint8_t& src) {
    if (_position >= _limit)
        throw ByteBufferException("ByteBuffer::put(): BufferOverflow");
    _hb[_position++] = src;
}

void ByteBuffer::put(const uint16_t& src) {
    put((uint8_t*) & src, 0, 2);
}

void ByteBuffer::put(const uint32_t& src) {
    put((uint8_t*) & src, 0, 4);
}

void ByteBuffer::put(const std::string& src) {
    put((uint8_t*) src.data(), 0, src.length());
}

void ByteBuffer::put(const char* src) {
    put((uint8_t*) src, 0, strlen(src));
}

uint8_t ByteBuffer::get(const size_t& index) const {
    checkBounds(index, 1, _capacity);
    return _hb[index];
}

uint8_t ByteBuffer::get() {
    return get(_position++);
}

ByteBuffer* ByteBuffer::allocate(const size_t& capacity) {
    if (capacity < 0)
        throw ByteBufferException("ByteBuffer::allocate(): IllegalArgument");
    return new ByteBuffer(0, capacity, capacity, new uint8_t[capacity]);
}

void ByteBuffer::freeMemery(void) {
    if (_hb) {
        delete _hb;
        _hb = NULL;
    }
}

ByteBuffer* ByteBuffer::wrap(uint8_t* array, const size_t& offset, const size_t& length, const size_t& capacity) {
    //checkBounds(offset, length, sizeof(array));
    try {
        new ByteBuffer(offset, offset + length, capacity, array);
    } catch (exception &e) {
        throw ByteBufferException("ByteBuffer::wrap(): IndexOutOfBounds");
    }
}

ByteBuffer* ByteBuffer::wrap(uint8_t* array, const size_t& capacity) {
    return wrap(array, 0, capacity, capacity);
}

size_t ByteBuffer::position(void) const {
    return _position;
}

size_t ByteBuffer::position(const size_t& newPosition) {
    if ((newPosition > _limit) || (newPosition < 0))
        throw ByteBufferException("ByteBuffer::movePosition(): IllegalArgument");
    _position = newPosition;
}

void ByteBuffer::movePosition(const size_t& length, const bool isReverse/* = false*/) {
    size_t newPos = isReverse ? (_position - length) : (_position + length);
    if ((newPos > _limit) || (newPos < 0))
        throw ByteBufferException("ByteBuffer::movePosition(): IllegalArgument");
    _position = newPos;
}

void ByteBuffer::modifyMemery(const uint8_t* src, const size_t& offset, const size_t& length, const size_t& position) {
    if (_position + length > _limit)
        throw ByteBufferException("ByteBuffer::modifyMemery(): IllegalArgument");
    for (size_t i = 0; i < length; i++)
        _hb[i + position] = src[offset + i];
}

const uint8_t& ByteBuffer::at(const size_t& position) const {
    if (position > _limit)
        throw ByteBufferException("ByteBuffer::at(): IllegalArgument");
    return _hb[position];
}

ByteBuffer* ByteBuffer::copy(void) const {
    if (0 >= _capacity || NULL == _hb)
        return NULL;
    ByteBuffer* copy = new ByteBuffer(_position, _limit, _capacity, new uint8_t[_capacity]);
    memcpy(_hb, copy->_hb, _capacity);
    return copy;
}

uint8_t* ByteBuffer::array(void) const {
    if (_hb == NULL)
        throw ByteBufferException("ByteBuffer::array(): UnsupportedOperation");
    return _hb;

}

string ByteBuffer::getString() const {
    return getString(0, remaining());
}

string ByteBuffer::getString(const size_t& offset, const size_t& length) const {
    checkBounds(offset, length, remaining());
    string str((char*) &_hb[_position + offset], length);
    return str;
}

void ByteBuffer::outputAsHex(ostream& out) const {
    outputAsHex(out, 0, remaining());
}

void ByteBuffer::outputAsHex(ostream& out, const size_t& offset, const size_t& length) const {
    checkBounds(offset, length, remaining());
    for (size_t i = 0; i < length; i++)
        out << hex << (short) _hb[_position + offset + i] << ' ';
    //        out << hex << setw(3) << setfill(' ') << (short) _hb[_position + offset + i];
}
//
//uint8_t* ByteBuffer::arrayOfPosition() const {
//    return array() + _position;
//}

void ByteBuffer::outputAsDec(ostream& out) const {
    outputAsDec(out, 0, remaining());
}

void ByteBuffer::outputAsDec(ostream& out, const size_t& offset, const size_t& length) const {
    checkBounds(offset, length, remaining());
    for (size_t i = 0; i < length; i++)
        //        out << (short) _hb[_position + offset + i] << ' ';
        out << setw(3) << setfill(' ') << (short) _hb[_position + offset + i];
}
