/* TString.h: Davicom TString header.                                           */
/*                                                                              */
/*==============================================================================*/
/* Header Name	: TString.h                                                     */
/* Purpose      : Davicom tstring for all devices.                              */
/* Autor        : Richard Chuang                                                */
/* Email        : richard_chung@davicom.com.tw                                  */
/* Description  :                                                               */
/* Reference    :                                                               */
/* Copyright    : (c) Davicom Semiconductor Inc. All Rights Reserved.           */
/* -----------------------------------------------------------------------------*/
/* Version      : 1.0.0.0                                                       */
/* Update       : 2017-12-14                                                    */
/* Modified     : Richard Chung                                                 */
/* Description	:                                                               */
/* -----------------------------------------------------------------------------*/
/* How to use:                                                                  */
/*                                                                              */
#ifndef _TSTRING_H_
#define _TSTRING_H_

using namespace std ;
#include <string>

#ifdef _UNICODE
#define StlString	wstring
#define SString		wstring
#define TString		wstring
#else
#define StlString	string
#define SString		string
#define TString		string
#endif

#include <vector>
class TStringArray : public vector< TString > {};

#endif	//_TSTRING_H_
