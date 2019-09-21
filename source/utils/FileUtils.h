// ******************************************************************************
// Filename:    FileUtils.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   Some general file and folder utility functions.
//
// Revision History:
//   Initial Revision - 17/03/13
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==nullptr
#endif

string wchar_t2string(const wchar_t *wchar);
wchar_t *string2wchar_t(const string &str);
vector<string> listFilesInDirectory(string directoryName);
