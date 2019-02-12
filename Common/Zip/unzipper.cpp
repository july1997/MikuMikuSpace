#include "unzipper.h"
#include <zlib.h>
#include <algorithm>
#include <sstream>
#include <thread>

namespace ziputils
{
#define for if (0) ; else for
#define IsShiftJIS(x) ((BYTE)((x ^ 0x20) - 0xA1) <= 0x3B)

    //---------------------------------------------------------------------------
    // ファイルが存在するかどうか
    bool unzipper::IsFileExist(const string &strFilename)
    {
        return GetFileAttributes(strFilename.c_str()) != 0xffffffff;
    }

    //---------------------------------------------------------------------------
    // 再帰的にディレクトリを作成（strPathの末尾には必ず\をつけること）
    bool unzipper::CreateDirectoryReflex(const string &strPath)
    {
        const char *p = strPath.c_str();

        for (; *p; p += IsShiftJIS(*p) ? 2 : 1)
            if (*p == '\\')
            {
                string strSubPath(strPath.c_str(), p);

                if (!IsFileExist(strSubPath.c_str()))
                    if (!CreateDirectory(strSubPath.c_str(), NULL))
                    {
                        return false;
                    }
            }

        return true;
    }

    //---------------------------------------------------------------------------
    /* unz_file_info_interntal contain internal info about a file in zipfile*/
    typedef struct unz_file_info64_internal_s
    {
        ZPOS64_T offset_curfile;/* relative offset of local header 8 bytes */
    } unz_file_info64_internal;
    /* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
    typedef struct
    {
        char  *read_buffer;         /* internal buffer for compressed data */
        z_stream stream;            /* zLib stream structure for inflate */

#ifdef HAVE_BZIP2
        bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

        ZPOS64_T pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
        uLong stream_initialised;   /* flag set if stream structure is initialised*/

        ZPOS64_T offset_local_extrafield;/* offset of the local extra field */
        uInt  size_local_extrafield;/* size of the local extra field */
        ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
        ZPOS64_T total_out_64;

        uLong crc32;                /* crc32 of all data uncompressed */
        uLong crc32_wait;           /* crc32 we must obtain after decompress all */
        ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
        ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
        zlib_filefunc64_32_def z_filefunc;
        voidpf filestream;        /* io structore of the zipfile */
        uLong compression_method;   /* compression method (0==store) */
        ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
        int   raw;
    } file_in_zip64_read_info_s;
    typedef struct
    {
        zlib_filefunc64_32_def z_filefunc;
        int is64bitOpenFunction;
        voidpf filestream;        /* io structore of the zipfile */
        unz_global_info64 gi;       /* public global information */
        ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
        ZPOS64_T num_file;             /* number of the current file in the zipfile*/
        ZPOS64_T pos_in_central_dir;   /* pos of the current file in the central dir*/
        ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
        ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

        ZPOS64_T size_central_dir;     /* size of the central directory  */
        ZPOS64_T offset_central_dir;   /* offset of start of central directory with
									   respect to the starting disk number */

        unz_file_info64 cur_file_info; /* public info about the current file in zip*/
        unz_file_info64_internal cur_file_info_internal; /* private info about it*/
        file_in_zip64_read_info_s* pfile_in_zip_read; /* structure about the current
													  file if we are decompressing it */
        int encrypted;

        int isZip64;

#    ifndef NOUNCRYPT
        unsigned long keys[3];     /* keys defining the pseudo-random sequence */
        const z_crc_t* pcrc_32_tab;
#    endif
    } unz64_s;
    //----------------------------------------------------------------------------------------------

    //フォルダ削除 関係------------------------------------------------------------------------------
    //この関数の引数fileNameは\で終わってはいけません。
    bool unzipper::removeDirectory(std::string fileName)
    {
        bool retVal = true;
        std::string nextFileName;
        WIN32_FIND_DATA foundFile;
        HANDLE hFile = FindFirstFile((fileName + "\\*.*").c_str(), &foundFile);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            do
            {
                //If a found file is . or .. then skip
                if (strcmp(foundFile.cFileName, ".") != 0 && strcmp(foundFile.cFileName, "..") != 0)
                {
                    //The path should be absolute path
                    nextFileName = fileName + "\\" + foundFile.cFileName;

                    //If the file is a directory
                    if ((foundFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                    {
                        removeDirectory(nextFileName.c_str());
                        RemoveDirectory(nextFileName.c_str());
                    }
                    //If the file is a file
                    else
                    {
                        DeleteFile(nextFileName.c_str());
                    }
                }
            }
            while (FindNextFile(hFile, &foundFile) != 0);
        }

        FindClose(hFile);

        //Delete starting point itseft
        if (RemoveDirectory(fileName.c_str()) == 0) { retVal = false; }

        return retVal;
    }
    //----------------------------------------------------------------------------------------------


    unzipper::unzipper()
    {
    }

    unzipper::~unzipper()
    {
    }

    bool unzipper::openZip(string path, string outfiledir)
    {
        if(outfiledir != "" )strTargetPath = outfiledir + "/";
		openFilePath = path;
        hUnzip = unzOpen(path.c_str());
        unz64_s* s;
        s = (unz64_s*)hUnzip;
        zipfilesize = s->offset_central_dir;

        if (!hUnzip) { return false; }

        return true;
    }

    void unzipper::startunzip()
    {
        std::thread th(&unzipper::unzip, this);
        // thread start
        th.detach();
    }

    void unzipper::unzip()
    {
		if (strTargetPath != "") {
			// すでにディレクトリがあれば削除
			removeDirectory(strTargetPath.c_str());

			//出力先ディレクトリ作成
			CreateDirectory(strTargetPath.c_str(), NULL);
		}

        while (1)
        {
            char szConFilename[512];
            unz_file_info fileInfo;

            if (unzGetCurrentFileInfo(hUnzip, &fileInfo, szConFilename, sizeof szConFilename, NULL, 0, NULL, 0) != UNZ_OK)
            {
                break;
            }

            int nLen = strlen(szConFilename);

            for (int i = 0; i < nLen; ++i)
                if (szConFilename[i] == '/')
                {
                    szConFilename[i] = '\\';
                }

            // ディレクトリの場合
            //if (nLen >= 2 && !IsShiftJIS(szConFilename[nLen - 2]) && szConFilename[nLen - 1] == '\\') {
            //	CreateDirectoryReflex(strTargetPath + szConFilename);
            //	continue;
            //}

            // ファイルの場合
            if (unzOpenCurrentFile(hUnzip) != UNZ_OK) { break; }

            CreateDirectoryReflex(strTargetPath + szConFilename);
            HANDLE hFile = CreateFile((strTargetPath + szConFilename).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
            BYTE szBuffer[8192];
            DWORD dwSizeRead;

            while ((dwSizeRead = unzReadCurrentFile(hUnzip, szBuffer, sizeof szBuffer)) > 0)
            {
                WriteFile(hFile, szBuffer, dwSizeRead, &dwSizeRead, NULL);
            }

            FlushFileBuffers(hFile);
            CloseHandle(hFile);
            unzCloseCurrentFile(hUnzip);

            if (unzGoToNextFile(hUnzip) == UNZ_END_OF_LIST_OF_FILE)
            {
                unzClose(hUnzip);
                installed = 1;
                break;
            }

            nowfilesize += fileInfo.compressed_size;
        }

		// zipファイル削除
		DeleteFile(openFilePath.c_str());
    }

    size_t unzipper::getNoFilewSize()
    {
        return nowfilesize;
    }

    size_t unzipper::getZipFileSize()
    {
        return zipfilesize;
    }

    bool unzipper::isComplete()
    {
        return installed;
    }
};