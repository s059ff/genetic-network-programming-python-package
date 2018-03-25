#pragma once

#include <cstring>
#include <sstream>
#include <string>

template <
    typename Arg0 = int,
    typename Arg1 = int,
    typename Arg2 = int,
    typename Arg3 = int,
    typename Arg4 = int,
    typename Arg5 = int,
    typename Arg6 = int,
    typename Arg7 = int>
std::string format(
    const char *text,
    const Arg0 &arg0 = 0,
    const Arg1 &arg1 = 0,
    const Arg2 &arg2 = 0,
    const Arg3 &arg3 = 0,
    const Arg4 &arg4 = 0,
    const Arg5 &arg5 = 0,
    const Arg6 &arg6 = 0,
    const Arg7 &arg7 = 0)
{
    std::stringstream ss;

    auto len = strlen(text);
    for (int i = 0; i < len; i++)
    {
        if (text[i] == '{' && isdigit(text[i + 1]) && text[i + 2] == '}')
        {
            int n = text[i + 1] - '0';
            switch (n)
            {
            case 0:
                ss << arg0;
                break;
            case 1:
                ss << arg1;
                break;
            case 2:
                ss << arg2;
                break;
            case 3:
                ss << arg3;
                break;
            case 4:
                ss << arg4;
                break;
            case 5:
                ss << arg5;
                break;
            case 6:
                ss << arg6;
                break;
            case 7:
                ss << arg7;
                break;
            default:
                break;
            }
            i += 2;
        }
        else
        {
            ss << text[i];
        }
    }

    return ss.str();
}

template <
    typename Arg0 = int,
    typename Arg1 = int,
    typename Arg2 = int,
    typename Arg3 = int,
    typename Arg4 = int,
    typename Arg5 = int,
    typename Arg6 = int,
    typename Arg7 = int>
std::wstring format(
    const wchar_t *text,
    const Arg0 &arg0 = 0,
    const Arg1 &arg1 = 0,
    const Arg2 &arg2 = 0,
    const Arg3 &arg3 = 0,
    const Arg4 &arg4 = 0,
    const Arg5 &arg5 = 0,
    const Arg6 &arg6 = 0,
    const Arg7 &arg7 = 0)
{
    std::wstringstream ss;

    auto len = wcslen(text);
    for (int i = 0; i < len; i++)
    {
        if (text[i] == '{' && isdigit(text[i + 1]) && text[i + 2] == '}')
        {
            int n = text[i + 1] - '0';
            switch (n)
            {
            case 0:
                ss << arg0;
                break;
            case 1:
                ss << arg1;
                break;
            case 2:
                ss << arg2;
                break;
            case 3:
                ss << arg3;
                break;
            case 4:
                ss << arg4;
                break;
            case 5:
                ss << arg5;
                break;
            case 6:
                ss << arg6;
                break;
            case 7:
                ss << arg7;
                break;
            default:
                break;
            }
            i += 2;
        }
        else
        {
            ss << text[i];
        }
    }

    return ss.str();
}

template <typename... Args>
std::string format(
    const std::string &text,
    const Args &... args)
{
    return format(text.c_str(), args...);
}

template <typename... Args>
std::wstring format(
    const std::wstring &text,
    const Args &... args)
{
    return format(text.c_str(), args...);
}
