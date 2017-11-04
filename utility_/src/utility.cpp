
#include "utility.h"

using namespace std;

string gutility::timeToStr(const time_t& time, const string& format) {
    struct ::tm* timeTM;
    char strTime[100] = {0};

    timeTM = ::localtime(&time);
    ::strftime(strTime, sizeof (strTime) - 1, format.c_str(), timeTM);
    string timeStr(strTime);
    return timeStr;
}

string gutility::timeToStr(const time_t& time) {
    return gutility::timeToStr(time, gutility::defaultTimeFormat);
}

string gutility::timeToStr(const string& format) {
    return gutility::timeToStr(time(NULL), format);
}
string gutility::timeToStr() {
    return gutility::timeToStr(gutility::defaultTimeFormat);
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
