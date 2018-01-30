#include "utility.h"
#include <sstream>
#include <stdexcept>
#include <iomanip>

using namespace std;

const string gutility::default_time_format("%Y-%m-%d %H:%M:%S");

time_t gutility::str_2_time(const char* format, const char* src) {
    if (!format || !src) throw runtime_error(string("gutility::") + __func__ + ": invalid argument");
    tm TM;
#ifdef _LIBCPP_COMPILER_CLANG
    if (NULL != ::strptime(src, format, &TM))
#else
    istringstream ss(src);
    ss >> get_time(&TM, format);
    if (!ss.fail())
#endif
    {
        time_t r = mktime(&TM);
        if (r >= 0) return r;
    }
    throw runtime_error(string("gutility::") + __func__ + ": transfer fail, format: " + format + ", src: " + src);
}

string gutility::time(const time_t& time, const char* format){
    struct tm* timeTM;
    char strTime[1024] = {0};

    timeTM = localtime(&time);
    strftime(strTime, sizeof (strTime) - 1, format, timeTM);
    return string(strTime);
}

string gutility::time(const time_t& time, const string& format/* = util::default_time_format*/) {
    return gutility::time(time, format.c_str());
}

string gutility::now(const string& format/* = default_time_format*/) {
    return gutility::time(std::time(nullptr), format.c_str());
}

vector<string> gutility::str_split(const string& src, const char& separator) {
    vector<string> dest;
    size_t curr = 0;
    size_t next = 0;
    
    for (; curr < src.length();) {
        next = src.find(separator, curr);
        if (next == string::npos) {
            string tmp = str_trim(src.substr(curr));
            if (tmp.size() > 0)
                dest.push_back(tmp);
            break;
        }
        string&& tmp = str_trim(src.substr(curr, next - curr));
        if (tmp.size() > 0)
            dest.push_back(tmp);
        curr = next + 1;
    }

    return dest;
}

string gutility::str_trim(string&& src) {
    size_t offset = src.find_first_not_of(' ');
    if (offset == string::npos) return "";
    return src.substr(offset, src.find_last_not_of(' ') - offset + 1);
}
