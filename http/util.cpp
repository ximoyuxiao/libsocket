#include<util.h>
namespace my{
int SplitString(const std::string str,const std::string tag,std::vector<std::string>& splite){
        splite.clear(); // 清空 splite 向量

        if (str.empty()) {
            return 0;
        }

        size_t pos = 0;
        size_t len = str.length();
        size_t tagLen = tag.length();
        if(str.substr(0,tagLen) == tag){
            pos = tagLen;
        }
        while (pos < len) {
            size_t found = str.find(tag, pos);

            if (found == std::string::npos) {
                // 如果没找到分隔符，说明当前是最后一个子串
                splite.push_back(str.substr(pos, len - pos));
                break;
            }

            // 如果找到了分隔符，说明当前子串是从 pos 到 found 的子串
            splite.push_back(str.substr(pos, found - pos));

            // 跳过分隔符
            pos = found + tagLen;
        }

        return splite.size();
    }

std::string URLdecode(std::string url){
    return url;
}
std::string URLencode(std::string url){
    return url;
}
};


