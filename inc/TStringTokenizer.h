#ifndef _TSTRING_TOKENIZER_
#define _TSTRING_TOKENIZER_

#include "TString.h"

class TStringTokenizer
{
public:
	TStringTokenizer();
	~TStringTokenizer();
	TStringArray Token(const TString strString, char szDelimiter);
	TStringArray Token(const TString strString, char* lpszDelimiter);
	void split(const string& s, vector<string>& tokens, const string& delimiters = " ");
};
#endif
