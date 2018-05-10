#if !defined(ZIKEN_STRINGFUNCS_INCLUDE_)
#define ZIKEN_STRINGFUNCS_INCLUDE_

#pragma warning(disable: 4996)
#pragma warning(disable: 4786)
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#ifdef _MSC_FULL_VER
#include <codecvt>
#else
#include <locale>
#endif

using namespace std;

#include "basictype.h"

namespace ziken
{

#ifdef _MSC_FULL_VER
static wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

static string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
#endif

static string trimleft(const string& s)
{
    long offset = -1;
    long size = s.size();
    for (long i = 0; i < size; i++)
    {
        if (' ' != s[i])
        {
            offset = i;
            break;
        }
    }

    if (0 > offset)
    {
        return string("");
    }

    return s.substr(offset, size);
}


static string trimright(const string& s)
{
    long offset = -1;
    for (long i = s.size() - 1; i >= 0; i--)
    {
        if (' ' != s[i])
        {
            offset = i;
            break;
        }
    }

    if (0 > offset)
    {
        return string("");
    }

    return s.substr(0, offset + 1);
}

static string trim(const string& s)
{
    return trimleft(trimright(s));
}

static string trim_line(const string& line)
{
    if (line.size() < 1)
        return "";

    string r = line;
    if (r[0] == '\n' || r[0] == '\r')
    {
        r = r.substr(1, r.size() - 1);
    }

    if (r.size() < 1)
        return "";

    if (r[r.size() - 1] == '\n' || r[r.size() - 1] == '\r')
    {
        r = r.substr(0, r.size() - 1);
    }

    return r;
}

static long FindFirstChar(const char* pc, const char find)
{
    char* p = (char*)pc;
    long offset = -1;
    while (0 != *p)
    {
        if (find == *p)
        {
            offset = p - pc;
            break;
        }
        p++;
    }

    return offset;
}

class Strings
{
public:
    Strings(){};
    Strings(const string& text){this->SetText(text);};
    ~Strings(){};

private:
    vector<string> m_lines;

public:
    void SetText(const string& text)
    {
        m_lines.clear();
        string sub = text;
        while (sub != "")
        {
            long rt = FindFirstChar(sub.c_str(), '\n');
            if (0 > rt)
            {
                if (sub.size() > 0)
                {
                    m_lines.push_back(sub);
                }
                break;
            }

            string line = trim_line(sub.substr(0, rt));
            m_lines.push_back(line);
            sub = sub.substr(rt + 1, sub.size() - 1);
        }
    };

    string GetText() const
    {
        vector<string>::const_iterator it = m_lines.begin();
        if (it == m_lines.end())
        {
            return "";
        }

        string text = *(it++);
        dword count = m_lines.size();
        for (dword i = 1; i < count; i++)
        {
            text = text + "\n" + *(it++);
        }

        return text;
    };

    dword GetLineCount() const
    {
        return m_lines.size();
    };

    bool GetLine(const dword index, string& line) const
    {
        if (m_lines.size() <= index)
        {
            return false;
        }

        line = m_lines[index];
        return true;
    };

    void AppendLine(const string& line)
    {
        Strings strings(line);
        for (dword i = 0; i < strings.GetLineCount(); i++)
        {
            string l;
            strings.GetLine(i, l);
            m_lines.push_back(l);
        }
    };

//    bool InsertLine(const dword index, const string& line);
    bool SetLine(const dword index, const string& line)
    {
        if (m_lines.size() <= index)
        {
            return false;
        }

        Strings strings(line);
        if (strings.GetLineCount() <= 0)
        {
            return false;
        }

        string l;
        strings.GetLine(0, l);
        m_lines[index] = l;
        return true;
    };

    bool DeleteLine(const dword index)
    {
        if (m_lines.size() <= index)
        {
            return false;
        }

        vector<string>::iterator it = m_lines.begin();
        for (dword i = 0; i < index; i++)
        {
            it++;
        }

        m_lines.erase(it);
        return true;
    };
};

static bool ini_setkeyvalue(string& text, const string& key, const string& value)
{
    Strings strings(text);

    long linecount = strings.GetLineCount();
    if (0 < linecount)
    {
        for (long i = 0; i< linecount; i++)
        {
            string line;
            strings.GetLine(i, line) ;
            long valuestart = key.size();
            string linehead = line.substr(0, valuestart);
            if (linehead == key)
            {
                strings.DeleteLine(i);
                break;
            }
        }
    }

    string keyval = key + " = " + value;
    strings.AppendLine(keyval);
    text = strings.GetText();
    return true;
}

static bool ini_findkeyvalue(const string& text, const string& key, string& value)
{
    Strings strings(text);

    bool r = false;
    long linecount = strings.GetLineCount();
    if (0 < linecount)
    {
        for (long i = 0; i< linecount; i++)
        {
            string line;
            strings.GetLine(i, line) ;
            long valuestart = key.size();
            string linehead = line.substr(0, valuestart);
            if (linehead == key)
            {
                value = trim(line.substr(valuestart, line.size()));
                value = trim(value.substr(1, value.size()));
                r = true;
                break;
            }
        }
    }

    return r;
}

static bool ini_deletekey(string& text, const string& key)
{
    Strings strings(text);
    bool r = false;

    long linecount = strings.GetLineCount();
    if (0 < linecount)
    {
        for (long i = 0; i< linecount; i++)
        {
            string line;
            strings.GetLine(i, line) ;
            long valuestart = key.size();
            string linehead = line.substr(0, valuestart);
            if (linehead == key)
            {
                strings.DeleteLine(i);
                r = true;
                break;
            }
        }
    }

    if (r)
    {
        text = strings.GetText();
    }

    return r;
}

static string UpperString(const string& s)
{
    string t = s;
//    return ::strupr((char*)t.c_str());
    transform(t.begin(), t.end(), t.begin(), ::toupper);
    return t;
}

static string LowerString(const string& s)
{
    string t = s;
//    return ::strlwr((char*)t.c_str());
    transform(t.begin(), t.end(), t.begin(), ::tolower);
    return t;
}

static string int2str(const long a)
{
    char buf[20];
    ::memset(buf, 0, sizeof(buf));
    sprintf(buf, "%03d", a);
    return buf;
}

static string IntToStr(const long i)
{
//    static char cBuff[10];
//    ::itoa(i, cBuff, 10);
//    return string(cBuff);
    stringstream ss;
    ss << i;
    return ss.str();
}

static long StrToInt(const string& s)
{
    return ::atoi(s.c_str());
}

static string FloatToStr(const double f)
{
//    static char cBuff[100];
//    ::sprintf(cBuff, "%f", f);
//    return string(cBuff);
    stringstream ss;
    ss << f;
    return ss.str();
}

static string SingleToStr(const float f)
{
    stringstream ss;
    ss << f;
    return ss.str();
}

static double StrToFloat(const string& s)
{
    return ::atof(s.c_str());
}

static long FindLastChar(const char* pc, const char find)
{
    char* p = (char*)pc;
    long offset = -1;
    while (0 != *p)
    {
        if (find == *p) offset = p - pc;
        p++;
    }

    return offset;
}

static float strings_getf(const Strings &strings, const int index)
{
    string line;
    strings.GetLine(index, line);
    return (float)StrToFloat(line);
}

static string GetDirectoryPart(const string& pathfilename)
{
    if (pathfilename.size() == 0)
        return string("");

    long offset0 = FindLastChar(pathfilename.c_str(), '\\');
    long offset1 = FindLastChar(pathfilename.c_str(), '/');
    long offset = max(offset0, offset1);
    if (offset < 0)
    {
        return string("");
    }
    else if (offset == 0)
    {
        return "/";
    }

    return string(pathfilename.substr(0, offset));
}

static string RemoveDirectoryPart(const string& pathfilename)
{
    long offset0 = FindLastChar(pathfilename.c_str(), '\\');
    long offset1 = FindLastChar(pathfilename.c_str(), '/');
    long offset = max(offset0, offset1);
    if (offset < 0)
    {
        return pathfilename;
    }

    return string(pathfilename.substr(offset + 1, pathfilename.length()));
}

static string GetExtNamePart(const string& filename)
{
    long offset = FindLastChar(filename.c_str(), '.');
    if (0 > offset)
    {
        return string("");
    }

    return string(filename.substr(offset + 1, filename.length()));
}

static string RemoveExtNamePart(const string& filename)
{
    long offset = FindLastChar(filename.c_str(), '.');
    if (0 > offset)
    {
        return filename;
    }

    return string(filename.substr(0, offset));
}

static bool File2String(const string& filename, string& text)
{
    ifstream ifs(filename.c_str(), ios_base::in);
    if (!ifs.is_open())
    {
        return false;
    }

    ostringstream buf;
    char c;
    while (buf && ifs.get(c))
    {
        buf.put(c);
    }

    text = buf.str();
    return true;
}

static bool String2File(const string& text, const string& filename)
{
    ofstream ofs(filename.c_str(), ios_base::out);
    if (!ofs.is_open())
    {
        return false;
    }

    long count = text.size();
    for (long i = 0; i < count; i++)
    {
        char c = text[i];
        ofs.put(c);
    }

    return true;
}

static vector<string> split(const string &s, const string &seperator)
{
  vector<string> result;
  typedef string::size_type string_size;
  string_size i = 0;
  
  while(i != s.size()){
    int flag = 0;
    while(i != s.size() && flag == 0){
        flag = 1;
        for(string_size x = 0; x < seperator.size(); ++x)
        if(s[i] == seperator[x]){
            ++i;
            flag = 0;
            break;
        }
    }
    
    flag = 0;
    string_size j = i;
    while(j != s.size() && flag == 0){
        for(string_size x = 0; x < seperator.size(); ++x)
        if(s[j] == seperator[x]){
            flag = 1;
            break;
        }
        if(flag == 0) 
        ++j;
    }

    if(i != j){
        result.push_back(s.substr(i, j-i));
        i = j;
    }
  }
  return result;
}

static bool comparestr(const string &str1, const string &str2)
{
    if (str1.compare(str2) == 0)
        return true;

    return false;
}

}

#endif
