#pragma once

#ifndef __ZIPHELPER_H_
#define __ZIPHELPER_H_

#include "CommonHelper.h"
#include <zip.h>
#include <unzip.h>

namespace PPSHUAI{

	BOOL CompressZipFile(LPCTSTR lpCompressFileName, LPCTSTR lpCompressFilePath, LPSTR lpPassword = (""), LPTSTR lpResult = NULL, DWORD dwLength = MAXBYTE)
	{
		BOOL bResult = FALSE;
		HZIP hZip = NULL;
		ZZRESULT zResult = ZZ_ERRNO;
		ZTCHAR tResult[MAXBYTE] = { 0 };
		TSTRINGTSTRINGMAP ttmap;
		PPSHUAI::FilePath::DirectoryTraversal(&ttmap, lpCompressFilePath, lpCompressFilePath);
		if (ttmap.size())
		{
			hZip = CreateZip(lpCompressFileName, lpPassword);
			if (hZip)
			{
				for (auto it : ttmap)
				{
					if (*it.second.rbegin() != Z_T('\\'))
					{
						zResult = ZipAdd(hZip, it.second.c_str(), it.first.c_str());
					}
					else
					{
						zResult = ZipAddFolder(hZip, it.second.c_str());
					}
					if (zResult != ZZR_OK)
					{
						break;
					}
				}

				bResult = (zResult == ZZR_OK);

				CloseZip(hZip);
				hZip = NULL;

				if (lpResult)
				{
					ZFormatZipMessageZ(zResult, lpResult, dwLength);
				}
			}
		}

		return bResult;
	}
	BOOL CompressZipFile(LPCTSTR lpCompressFileName, TSTRINGVECTOR tsv, LPSTR lpPassword = (""), LPTSTR lpResult = NULL, DWORD dwLength = MAXBYTE)
	{
		BOOL bResult = FALSE;
		HZIP hZip = NULL;
		ZZRESULT zResult = ZZ_ERRNO;
		ZTCHAR tResult[MAXBYTE] = { 0 };

		hZip = CreateZip(lpCompressFileName, lpPassword);
		if (hZip && tsv.size())
		{
			ZTCHAR tFileName[MAX_PATH] = { 0 };
			ZTCHAR tExtended[MAX_PATH] = { 0 };
			for (auto it : tsv)
			{
				z_tsplitpath(it.c_str(), NULL, NULL, tFileName, tExtended);
				z_tcscat(tFileName, (*tExtended) ? tExtended : Z_T(""));
				
				zResult = ZipAdd(hZip, tFileName, it.c_str());
				if (zResult != ZZR_OK)
				{
					break;
				}
			}
		
			bResult = (zResult == ZZR_OK);
			
			CloseZip(hZip);
			hZip = NULL;
			
			if (lpResult)
			{
				ZFormatZipMessageZ(zResult, lpResult, dwLength);
			}
		}

		return bResult;
	}
	BOOL UnCompressZipFile(LPCTSTR lpUnCompressPathName, LPCTSTR lpCompressFileName, LPSTR lpPassword = (""), LPTSTR lpResult = NULL, DWORD dwLength = MAXBYTE)
	{
		BOOL bResult = FALSE;
		HZIP hZip = NULL;
		ZIPENTRY ze = { 0 };
		ZZRESULT zResult = ZZ_ERRNO;

		hZip = OpenZip(lpCompressFileName, lpPassword);
		if (hZip)
		{			
			zResult = SetUnzipBaseDir(hZip, lpUnCompressPathName);
			if (zResult == ZZR_OK)
			{
				zResult = GetZipItem(hZip, -1, &ze);
				if (ze.index > 0)
				{
					for (int i = 0; i < ze.index; i++)
					{
						zResult = GetZipItem(hZip, i, &ze);
						if (zResult == ZZR_OK)
						{
							zResult = UnzipItem(hZip, i, ze.name);
						}
						if (zResult != ZZR_OK)
						{
							break;
						}
					}
				}
			}
						
			bResult = (zResult == ZZR_OK) ? TRUE : FALSE;

			CloseZip(hZip);
			hZip = NULL;

			if (lpResult)
			{
				ZFormatZipMessageU(zResult, lpResult, dwLength);
			}
		}
		return bResult;
	}
	BOOL UnCompressZipFile(LPCTSTR lpUnCompressPathName, LPCVOID lpCompressData, DWORD dwCompressDataSize, LPSTR lpPassword = (""), LPTSTR lpResult = NULL, DWORD dwLength = MAXBYTE)
	{
		BOOL bResult = FALSE;
		HZIP hZip = NULL;
		ZIPENTRY zeRoot = { 0 };
		ZIPENTRY zeChild = { 0 };
		ZZRESULT zResult = ZZ_ERRNO;

		hZip = OpenZip((void *)lpCompressData, dwCompressDataSize, lpPassword);
		if (hZip)
		{			
			zResult = SetUnzipBaseDir(hZip, lpUnCompressPathName);
			if (zResult == ZZR_OK)
			{
				zResult = GetZipItem(hZip, -1, &zeRoot);
				if (zeRoot.index > 0)
				{
					for (int i = 0; i < zeRoot.index; i++)
					{
						zResult = GetZipItem(hZip, i, &zeChild);
						if (zResult == ZZR_OK)
						{
							zResult = UnzipItem(hZip, i, zeChild.name);
						}
						if (zResult != ZZR_OK)
						{
							break;
						}
					}
				}
			}
						
			bResult = (zResult == ZZR_OK) ? TRUE : FALSE;

			CloseZip(hZip);
			hZip = NULL;

			if (lpResult)
			{
				ZFormatZipMessageU(zResult, lpResult, dwLength);
			}
		}
		return bResult;
	}
}

#endif //__ZIPHELPER_H_