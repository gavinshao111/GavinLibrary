
/* 
 * File:   utility.h
 * Author: 10256
 *
 * Created on 2017年7月25日, 下午1:45
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>
#include <ctime>

namespace gutility {
    extern const std::string default_time_format;
    
    std::time_t str_2_time(const char* format, const char* src);
    std::string time(const time_t& time, const std::string& format = default_time_format);
    std::string time(const time_t& time, const char* format);
    std::string now(const std::string& format = default_time_format);
    
    std::vector<std::string> str_split(const std::string& src, const char& separator);
    std::string str_trim(std::string&& src);
}


#endif /* UTILITY_H */

