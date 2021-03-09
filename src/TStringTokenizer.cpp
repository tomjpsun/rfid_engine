#include <string>
#include "TStringTokenizer.h"


TStringTokenizer::TStringTokenizer()
{
}


TStringTokenizer::~TStringTokenizer()
{
}

TStringArray TStringTokenizer::Token(const TString strString, char szDelimiter)
{
	TStringArray aryStrings;
#ifdef _UNICODE
	std::wstring::size_type nPrevPos = 0, nPos = 0;
#else
	std::string::size_type nPrevPos = 0, nPos = 0;
#endif

#ifdef _UNICODE
	while ((nPos = strString.find(szDelimiter, nPos)) != std::wstring::npos)
#else
	while ((nPos = strString.find(szDelimiter, nPos)) != std::string::npos)
#endif

	{
		TString strSubstring(strString.substr(nPrevPos, nPos - nPrevPos));
		aryStrings.push_back(strSubstring);
		nPrevPos = ++nPos;
	}

	aryStrings.push_back(strString.substr(nPrevPos, nPos - nPrevPos)); // Last word
	return aryStrings;
}

TStringArray TStringTokenizer::Token(const TString strString, char* lpszDelimiter)
{
	TStringArray aryStrings;
#ifdef _UNICODE
	std::wstring::size_type nPrevPos = 0, nPos = 0;
#else
	std::string::size_type nPrevPos = 0, nPos = 0;
#endif

#ifdef _UNICODE
	while ((nPos = strString.find(lpszDelimiter, nPos)) != std::wstring::npos)
#else
	while ((nPos = strString.find(lpszDelimiter, nPos)) != std::string::npos)
#endif

	{
		TString strSubstring(strString.substr(nPrevPos, nPos - nPrevPos));
		aryStrings.push_back(strSubstring);
		nPrevPos = ++nPos;
	}

	aryStrings.push_back(strString.substr(nPrevPos, nPos - nPrevPos)); // Last word
	return aryStrings;
}

void TStringTokenizer::split(const string& s, vector<string>& tokens, const string& delimiters)
{
	string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{
		tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}
#if 0
 // C++ 11 ´£¨Ñ¤F regex
void TStringTokenizer::regexSample()
{
	std::string text = "Quick brown fox.";
	std::regex ws_re("\\s+"); // whitespace
	std::vector<std::string> v(std::sregex_token_iterator(text.begin(), text.end(), ws_re, -1),
		std::sregex_token_iterator());for(auto&& s: v)
}
#endif
