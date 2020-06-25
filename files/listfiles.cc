/*
Copyright (C) 2001-2013 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdlib>
#include <cctype>
#include <cstdio>

#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#ifndef UNDER_EMBEDDED_CE
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::strcat;
using std::strcpy;
using std::strlen;
#endif

#include "utils.h"
#include "listfiles.h"


// System Specific Code for Windows
#if defined(WIN32)

// Need this for _findfirst, _findnext, _findclose
#include <windows.h>
#include <tchar.h>

int U7ListFiles(const std::string &mask, FileList &files) {
	string          path(get_system_path(mask));
	const TCHAR     *lpszT;
	WIN32_FIND_DATA fileinfo;
	HANDLE          handle;
	char            *stripped_path;
	int             i, nLen, nLen2;

#ifdef UNICODE
	const char *name = path.c_str();
	nLen = strlen(name) + 1;
	LPTSTR lpszT2 = reinterpret_cast<LPTSTR>(_alloca(nLen * 2));
	lpszT = lpszT2;
	MultiByteToWideChar(CP_ACP, 0, name, -1, lpszT2, nLen);
#else
	lpszT = path.c_str();
#endif

	handle = FindFirstFile(lpszT, &fileinfo);

	stripped_path = new char [path.length() + 1];
	strcpy(stripped_path, path.c_str());

	for (i = strlen(stripped_path) - 1; i; i--)
		if (stripped_path[i] == '\\' || stripped_path[i] == '/')
			break;

	if (stripped_path[i] == '\\' || stripped_path[i] == '/')
		stripped_path[i + 1] = 0;


#ifdef DEBUG
	std::cerr << "U7ListFiles: " << mask << " = " << path << std::endl;
#endif

	// Now search the files
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			nLen = std::strlen(stripped_path);
			nLen2 = _tcslen(fileinfo.cFileName) + 1;
			char *filename = new char [nLen + nLen2];
			strcpy(filename, stripped_path);
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, fileinfo.cFileName, -1, filename + nLen, nLen2, NULL, NULL);
#else
			std::strcat(filename, fileinfo.cFileName);
#endif

			files.push_back(filename);
#ifdef DEBUG
			std::cerr << filename << std::endl;
#endif
			delete [] filename;
		} while (FindNextFile(handle, &fileinfo));
	}

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		LPTSTR lpMsgBuf;
		char *str;
		FormatMessage(
		    FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM |
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    reinterpret_cast<LPTSTR>(&lpMsgBuf),
		    0,
		    NULL
		);
#ifdef UNICODE
		nLen2 = _tcslen(lpMsgBuf) + 1;
		str = reinterpret_cast<char *>(_alloca(nLen));
		WideCharToMultiByte(CP_ACP, 0, lpMsgBuf, -1, str, nLen2, NULL, NULL);
#else
		str = lpMsgBuf;
#endif
		std::cerr << "Error while listing files: " << str << std::endl;
		LocalFree(lpMsgBuf);
	}

#ifdef DEBUG
	std::cerr << files.size() << " filenames" << std::endl;
#endif

	delete [] stripped_path;
	FindClose(handle);
	return 0;
}


#elif defined(__MORPHOS__) || defined(AMIGA)

#define NO_PPCINLINE_VARARGS
#define NO_PPCINLINE_STDARG
#include <proto/dos.h>

static struct AnchorPath ap __attribute__((aligned(4)));

int U7ListFiles(const std::string &mask, FileList &files) {
	string path(get_system_path(mask));
	char   buffer[ 256 ];
	size_t pos;

	// convert MS-DOS jokers to AmigaDOS wildcards
	while ((pos = path.find('*')) != string::npos)
		path.replace(pos, 1, "#?");

	if (ParsePattern(path.c_str(), buffer, sizeof(buffer)) != -1) {
		LONG error = MatchFirst(buffer, &ap);

		while (error == DOSFALSE) {
			files.push_back(ap.ap_Info.fib_FileName);
			error = MatchNext(&ap);
		}

		MatchEnd(&ap);
	} else
		cout << "ParsePattern() failed." << endl;

	return 0;
}
#elif defined(GEKKO)
#include <sys/param.h> // for MAXPATHLEN

int U7ListFiles(const std::string &mask, FileList &files) {
	string path(get_system_path(mask));
	char dir_path[MAXPATHLEN];
	char filename[MAXPATHLEN];
	char ext[4];

	File_SplitPath(path.c_str(), dir_path, filename, ext);

	int count = 0;
	int maxfiles = 1000;
	struct dirent *dp = NULL;

	/* Open directory */
	DIR *dir = opendir(dir_path);
	if (dir == NULL)
	{
		cerr << "U7ListFiles : Error opening" << dir_path << endl;
		return 0;
	}

	/* list entries */
	while ((dp=readdir(dir)) != NULL  && (count < maxfiles))
	{
		if (dp->d_name[0] == '.' && strcmp(dp->d_name, "..") != 0 )
			continue;

		if (strstr(dp->d_name, ext))
		{
			files.push_back(dp->d_name);
		}
	}

	/* close directory */
	closedir(dir);

	return 0;
}


#else   // This system has glob.h

#include <glob.h>

int U7ListFiles(const std::string &mask, FileList &files) {
	glob_t globres;
	string path(get_system_path(mask));
	int err = glob(path.c_str(), GLOB_NOSORT, 0, &globres);


	switch (err) {
	case 0:   //OK
		for (size_t i = 0; i < globres.gl_pathc; i++) {
			files.push_back(globres.gl_pathv[i]);
		}
		globfree(&globres);
		return 0;
	case 3:   //no matches
		return 0;
	default:  //error
		cerr << "Glob error " << err << endl;
		return err;
	}
}

#endif
