
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
    const static std::string defaultTimeFormat("%Y-%m-%d %H:%M:%S");
    std::string timeToStr(const std::time_t& time, const std::string& format);
    std::string timeToStr(const std::string& format);
    std::string timeToStr(const std::time_t& time);
    std::string timeToStr();
    
    std::vector<std::string> str_split(const std::string& src, const char& separator);
    std::string str_trim(std::string&& src);
}


#endif /* UTILITY_H */

