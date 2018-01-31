
/* 
 * File:   h264parser.h
 * Author: 10256
 *
 * Created on 2017年12月29日, 下午1:35
 */

#ifndef H264PARSER_H
#define H264PARSER_H

#include <fstream>
#include "ByteBuffer.h"

class h264parser {
public:
    h264parser(const std::string& file_path);
    virtual ~h264parser();
    
    bytebuf::sharedptr_t frame();

    /**
     * find next start code(0001), and return the offset.
     *
     * @return -1 if doesn't contain any start coode
     */
    static int get_next_startcode(const uint8_t src[], const size_t& offset, const size_t& length);
    /**
     * find next start code(0001) start from current position + offset in next
     * length of bytes, and return the offset after next start code and
     * src.position move to the position after next start code
     */
    static int move_to_next_startcode(bytebuf::ByteBuffer& src, const size_t& offset, const size_t& length);
    /**
     * find next start code(0001) start from current position in remaining
     * bytes, and return the offset after next start code and src.position move
     * to the position after next start code
     */
    static int move_to_next_startcode(bytebuf::ByteBuffer& src);

    static size_t s_max_frame_size;
private:
    std::ifstream m_ifs;
    h264parser(const h264parser& orig);

    bytebuf::ByteBuffer m_stream_buf;

};

#endif /* H264PARSER_H */

