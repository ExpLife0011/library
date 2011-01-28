#include <assert.h>

#include <stdio.h>
#include <string.h>

#include <iostream>

#include "encode.hpp"


const char* TEST_STR = "��C/C++��������wchar_t�ַ����ַ�������ȷ֧�̶ֳ�";
const wchar_t* TEST_WSTR = L"��C/C++��������wchar_t�ַ����ַ�������ȷ֧�̶ֳ�";
// const char* TEST_STR = "��ͳ��C�����ַ��������ַ�Ϊ��β��һϵ���ֽڵļ��ϣ���һ����ԪΪchar���ͣ�\
// ���Ա��������ϵļ����ֲ�ͬ���ַ���������ASCII��ISO-8859��GBK��BIG-5��SHIFT-JIS��\
// UTF-8�ȵȡ�ͨ����Щ�ַ������Ǽ���ASCII�ַ����ġ����ǿ��԰���Щ����ASCII�ַ���ͳ��\
// ΪANSI�ַ���������Ϊ���ֲ�ͬ���ַ����Ĵ��ڣ�������ʵ�ʵ�ʹ���л��������ȱ�ֺ�����\
// ������\
//         Unicode�ǰ��������ϸ������Ժͷ��ŵı��롣\
//         ��ʵ��Ӧ���У�Unicode��UTF-8��UTF-16��UTF-32��������ʽ���ڣ���������ʽ��������\
// ���ر�ʾUnicode�ĸ�����λ��\
//         ���磺���ĵ����֣���Unicode��λΪU+4E2D����UTF-8������8λ���ֽ�����ʾΪ��0xE4\
// 0xB8   0xAD��UTF-16��ʹ��һ��16λ������0x4E2D����ʾ��UTF-32��ʹ��һ��32λ������\
// 0x00004E2D����ʾ��(���ַ���λ����0xFFFFʱ��UTF-8Ҫ���ĸ�8λ���ֽ�����ʾ��UTF-16��Ҫ\
// ʹ��2��16λ��������ʾ)\
//         UTF-16��UTF-32��ȣ�UTF-16�Եø��Ӽ�ࡣWindows���ձ�ʹ��UTF-16����Linux������\
// Unix���ϵͳ������ʷ�ϵ����ԭ����Ƚ�������ʹ��UTF-8��UTF-32�����ִ��䷽ʽ��\
//         C/C++��׼���Ѿ��ṩ��wchat_t�ؼ�����ʵ�ֶ�Unicode��֧�֣�����wchat_t����ֻ����\
// Unicode���롣��Windowsƽ̨�£�wchat_t������16λ�ģ�����Linux�£�wchat_t������32λ�ġ�";
// const wchar_t* TEST_WSTR = L"��ͳ��C�����ַ��������ַ�Ϊ��β��һϵ���ֽڵļ��ϣ���һ����ԪΪchar���ͣ�\
// ���Ա��������ϵļ����ֲ�ͬ���ַ���������ASCII��ISO-8859��GBK��BIG-5��SHIFT-JIS��\
// UTF-8�ȵȡ�ͨ����Щ�ַ������Ǽ���ASCII�ַ����ġ����ǿ��԰���Щ����ASCII�ַ���ͳ��\
// ΪANSI�ַ���������Ϊ���ֲ�ͬ���ַ����Ĵ��ڣ�������ʵ�ʵ�ʹ���л��������ȱ�ֺ�����\
// ������\
//         Unicode�ǰ��������ϸ������Ժͷ��ŵı��롣\
//         ��ʵ��Ӧ���У�Unicode��UTF-8��UTF-16��UTF-32��������ʽ���ڣ���������ʽ��������\
// ���ر�ʾUnicode�ĸ�����λ��\
//         ���磺���ĵ����֣���Unicode��λΪU+4E2D����UTF-8������8λ���ֽ�����ʾΪ��0xE4\
// 0xB8   0xAD��UTF-16��ʹ��һ��16λ������0x4E2D����ʾ��UTF-32��ʹ��һ��32λ������\
// 0x00004E2D����ʾ��(���ַ���λ����0xFFFFʱ��UTF-8Ҫ���ĸ�8λ���ֽ�����ʾ��UTF-16��Ҫ\
// ʹ��2��16λ��������ʾ)\
//         UTF-16��UTF-32��ȣ�UTF-16�Եø��Ӽ�ࡣWindows���ձ�ʹ��UTF-16����Linux������\
// Unix���ϵͳ������ʷ�ϵ����ԭ����Ƚ�������ʹ��UTF-8��UTF-32�����ִ��䷽ʽ��\
//         C/C++��׼���Ѿ��ṩ��wchat_t�ؼ�����ʵ�ֶ�Unicode��֧�֣�����wchat_t����ֻ����\
// Unicode���롣��Windowsƽ̨�£�wchat_t������16λ�ģ�����Linux�£�wchat_t������32λ�ġ�";

void Test()
{
    std::cout << std::endl << "WideToUtf8----------------:" << std::endl;
    {
        std::string str = util::WideToUtf8(TEST_WSTR);
        std::cout << TEST_STR << " -> " << str << std::endl;
        std::cout << "srclen=" << strlen(TEST_STR) << "    "
                  << "len=" << str.length() << std::endl;
    }

    std::cout << std::endl << "GbkToUtf8----------------:" << std::endl;
    {
        std::string str = util::GbkToUtf8(TEST_STR);
        std::cout << TEST_STR << " -> " << str << std::endl;
        std::cout << "srclen=" << strlen(TEST_STR) << "    "
                  << "len=" << str.length() << std::endl;
    }
#ifndef __MINGW32__
    std::cout << std::endl << "Utf8ToWide----------------:" << std::endl;
    {
		std::string utf8 = util::WideToUtf8(TEST_WSTR);
        std::wstring str = util::Utf8ToWide(utf8.c_str());
        // std::wcout << TEST_WSTR << " -> " << util::Utf8ToWide(util::WideToUtf8(TEST_WSTR).c_str()) << std::endl;
        std::wcout << TEST_WSTR << L" -> " << str << std::endl;
        std::cout << "srclen=" << utf8.length() << "    "
                  << "len=" << str.length() << std::endl;
    }
#endif
    std::cout << std::endl << "Utf8ToGbk----------------:" << std::endl;
    {
		std::string utf8 = util::WideToUtf8(TEST_WSTR);
        std::string str = util::Utf8ToGbk(utf8.c_str());
        // std::cout << TEST_STR << " -> " << util::Utf8ToGbk(util::WideToUtf8(TEST_WSTR).c_str()) << std::endl;
        std::cout << TEST_STR << " -> " << str << std::endl;
        std::cout << "srclen=" << utf8.length() << "    "
                  << "len=" << str.length() << std::endl;
    }
#ifndef __MINGW32__
    std::cout << std::endl << "GbkToWide----------------:" << std::endl;
    {
        std::wstring str = util::GbkToWide(TEST_STR);
        std::wcout << TEST_WSTR << L" -> " << str << std::endl;
        std::cout << "srclen=" << strlen(TEST_STR) << "    "
                  << "len=" << str.length() << std::endl;
    }
#endif
    std::cout << std::endl << "WideToGbk----------------:" << std::endl;
    {
        std::string str = util::WideToGbk(TEST_WSTR);
        std::cout << TEST_STR << " -> " << str << std::endl;
        std::cout << "srclen=" << wcslen(TEST_WSTR) << "    "
                  << "len=" << str.length() << std::endl;
    }
}


int main()
{
    Test();
#ifdef _WIN32
    getchar();
#endif
    return 0;
}
