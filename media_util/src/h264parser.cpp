
/* 
 * File:   h264parser.cpp
 * Author: 10256
 * 
 * Created on 2017年12月29日, 下午1:35
 */

#include "h264parser.h"
#include <stdexcept>

size_t h264parser::s_max_frame_size(0x400 * 500);

h264parser::h264parser(const std::string& file_path) :
m_stream_buf(s_max_frame_size * 30),
m_ifs(file_path, std::ofstream::in | std::ofstream::binary) {
    if (!m_ifs)
        throw std::runtime_error(std::string(__func__) + ": open " + file_path + " fail");

    m_ifs.read((char*) m_stream_buf.array(), m_stream_buf.remaining());
    m_stream_buf.limit(m_ifs.gcount());
}

h264parser::~h264parser() {
    if (m_ifs.is_open())
        m_ifs.close();
}

bytebuf::sharedptr_t h264parser::frame() {
    if (!m_stream_buf.hasRemaining()) {
        m_stream_buf.clear();
        if (m_ifs.eof()) {
            m_ifs.clear();
            m_ifs.seekg(std::ios::beg); // 重置指针到最前
        }
        m_ifs.read((char*) m_stream_buf.array(), m_stream_buf.remaining());
        m_stream_buf.limit(m_ifs.gcount());
    }

    size_t currentNaluPos = m_stream_buf.position();
    m_stream_buf.get();
    for (; move_to_next_startcode(m_stream_buf) != -1 && m_stream_buf.position() - currentNaluPos < 1000; m_stream_buf.get());
    if (m_stream_buf.position() - currentNaluPos > s_max_frame_size)
        return nullptr;
    return std::make_shared<bytebuf::ByteBuffer>(m_stream_buf.array(), currentNaluPos, m_stream_buf.position() - currentNaluPos);
}

/**
 * find next start code(0001), and return the offset.
 *
 * @return -1 if doesn't contain any start coode
 */
int h264parser::get_next_startcode(const uint8_t src[], const size_t& offset, const size_t& length) {
    size_t i = offset;
    for (; i < offset + length; ++i) {
        if (src[i] == 0
                && src[i + 1] == 0
                && src[i + 2] == 0
                && src[i + 3] == 1) {
            return i;
        }
    }
    return -1;
}

/**
 * find next start code(0001) start from current position + offset in next
 * length of bytes, and return the offset after next start code and
 * src.position move to the position after next start code if find the
 * start code, otherwise -1 is returned and src.position move to the end.
 */
int h264parser::move_to_next_startcode(bytebuf::ByteBuffer& src, const size_t& offset, const size_t& length) {
    if (length + offset > src.remaining()) {
        throw std::runtime_error(std::string(__func__) + ": Illegal arguement");
    }

    int rc = h264parser::get_next_startcode(src.array(), offset + src.position(), length);
    if (rc == -1) {
        return -1;
    }
    src.position(rc);
    return rc;
}

/**
 * find next start code(0001) start from current position in remaining
 * bytes, and return the offset after next start code and src.position move
 * to the position after next start code if find the start code, otherwise
 * -1 is returned and src.position move to the end.
 */
int h264parser::move_to_next_startcode(bytebuf::ByteBuffer& src) {
    return h264parser::move_to_next_startcode(src, 0, src.remaining());
}