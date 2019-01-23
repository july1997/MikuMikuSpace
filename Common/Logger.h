#pragma once
#include <sstream>
#include <string>
#include <time.h>

#define LOGD( ... ) { Logger::Write(_T(__FUNCTION__), NULL, " Log : ",__VA_ARGS__ ); }
#define LOGE( ... ) { Logger::Write(_T(__FUNCTION__), NULL, " Error : ",__VA_ARGS__ ); }

class Logger
{
        static std::string &getfilename()
        {
            static std::string filename;
            return (filename);
        }

        inline static void concat_internal(std::stringstream &sout)
        {
        }

        template<typename First, typename... Rest>
        static void concat_internal(std::stringstream &sout, const First& first, const Rest&... rest)
        {
            sout << first;
            concat_internal(sout, rest...);
        }

        static std::string gettime()
        {
            time_t curUTC_sec;
            struct tm tmLocal;
            char ascstr[32];
            // Get Current Time(UTC sec.)
            time(&curUTC_sec);
            // UTC sec. -> Local Time
            localtime_s(&tmLocal, &curUTC_sec);
            asctime_s(ascstr, sizeof(ascstr), &tmLocal);
            return std::string(ascstr);
        }

    public:

        static void Initialize(const std::string& fileName)
        {
            getfilename() = fileName;
        }

        template<typename... Args>
        static void Write(LPCTSTR lpszFuncName, LPCTSTR lpszFormat, const Args&... args)
        {
            std::ofstream outputfile(getfilename(), std::ios_base::app);
            std::stringstream sout;
            concat_internal(sout, args...);
            std::string now = gettime();
            outputfile << now.substr(0, now.length() - 1).c_str();
            outputfile << sout.str() << " [" << lpszFuncName << "]" << std::endl;
            outputfile.close();
        }

};
