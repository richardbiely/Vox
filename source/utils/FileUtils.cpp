// ******************************************************************************
// Filename:    FileUtils.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 17/03/13
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "FileUtils.h"

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#endif


string wchar_t2string(const wchar_t *wchar)
{
	string str = "";
	int index = 0;
	while(wchar[index] != 0)
	{
		str += (char)wchar[index];
		++index;
	}
	return str;
}

wchar_t *string2wchar_t(const string &str)
{
	static wchar_t wchar[260];
	unsigned int index = 0;
	while(index < str.size())
	{
		wchar[index] = (wchar_t)str[index];
		++index;
	}
	wchar[index] = 0;
	return wchar;
}

vector<string> listFilesInDirectory(string directoryName)
{
#ifdef _WIN32
	WIN32_FIND_DATA FindFileData;
	std::wstring widestr = std::wstring(directoryName.begin(), directoryName.end());
	const wchar_t * FileName = widestr.c_str();
	HANDLE hFind = FindFirstFile(FileName, &FindFileData);
	vector<string> listFileNames;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			listFileNames.push_back(wchar_t2string(FindFileData.cFileName));
		}

		while (FindNextFile(hFind, &FindFileData))
			listFileNames.push_back(wchar_t2string(FindFileData.cFileName));

		FindClose(hFind);
	}
	else
	{
		DWORD errorCode = GetLastError();
		cerr << "listFilesInDirectory() failed with error: " << errorCode << endl;
	}

	return listFileNames;
#elif __linux__
	directoryName = directoryName.substr(0, directoryName.length() - 3);
	vector<string> listFileNames;
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(directoryName.c_str())) == nullptr)
	{
		cout << "Error(" << errno << ") opening " << directoryName << endl;
		return listFileNames;
	}

	while ((dirp = readdir(dp)) != nullptr)
	{
		listFileNames.push_back(string(dirp->d_name));
	}

	closedir(dp);

	return listFileNames;
#endif //_WIN32
}
