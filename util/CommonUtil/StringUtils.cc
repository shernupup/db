#include<CommonUtil/StringUtils.h>

bool detail::endsWith(const std::string& s, const char* suffix, size_t suffix_size)
{
    return s.size() >= suffix_size && 0 == memcmp(s.data() + s.size() - suffix_size, suffix, suffix_size);
}


bool detail::startsWith(const std::string& s, const char* prefix, size_t prefix_size)
{
    return s.size() >= prefix_size && 0 == memcmp(s.data(), prefix, prefix_size);
}
