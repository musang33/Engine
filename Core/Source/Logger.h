#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <map>
#include <array>

#include <iostream>

#include <boost/chrono/io/time_point_io.hpp>
#include <boost/chrono/chrono.hpp>

#include <boost/format.hpp>
#include <boost/locale/generator.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;

static const int bufferSize = 1000;


enum XsELogLevel
{
	LEVEL_NONE = 0,
	LEVEL_DEBUG = 1,
	LEVEL_INFO = 2,
	LEVEL_WARN = 3,
	LEVEL_ERROR = 4,
	LEVEL_CRITICAL = 5,
	LEVEL_DISABLE = 6,
	LOG_LEVEL_SIZE = 7
};

class XsContentsLogLevelManager
{
private:
	typedef map<std::string, XsELogLevel>					FolderNameByLogLevelMap;
	FolderNameByLogLevelMap									folderNameByLogLevelMap_;

	std::string												rootFolderName_;
	std::array<std::string, XsELogLevel::LOG_LEVEL_SIZE>	logLevelEnumToString_;

private:
	void TestCode()
	{
		rootFolderName_ = std::string( "inshalla" );

		folderNameByLogLevelMap_.insert( std::pair<std::string, XsELogLevel>(
			std::string( "source\\" ), XsELogLevel::LEVEL_DEBUG ) );
		folderNameByLogLevelMap_.insert( std::pair<std::string, XsELogLevel>(
			std::string( "source\\logger\\kkk\\" ), XsELogLevel::LEVEL_CRITICAL ) );
		folderNameByLogLevelMap_.insert( std::pair<std::string, XsELogLevel>(
			std::string( "source\\x\\" ), XsELogLevel::LEVEL_CRITICAL ) );

		logLevelEnumToString_ = {
			std::string( "LEVEL_NONE" ),
			std::string( "LEVEL_DEBUG" ),
			std::string( "LEVEL_INFO" ),
			std::string( "LEVEL_WARN" ),
			std::string( "LEVEL_ERROR" ),
			std::string( "LEVEL_CRITICAL" ),
			std::string( "LEVEL_DISABLE" )
		};
	}

public:
	XsContentsLogLevelManager()
	{
		TestCode();
	}

	const std::string& GetRootFolderName()
	{
		return rootFolderName_;
	}

	const std::string& GetLogLevelToString( XsELogLevel xsELogLevel )
	{
		return logLevelEnumToString_[xsELogLevel];
	}

	bool IsPrintable( XsELogLevel _logLevel, const std::string& _filePathFromRoot )
	{
		for( auto rit = folderNameByLogLevelMap_.rbegin(); rit != folderNameByLogLevelMap_.rend(); ++rit )
		{
			if( 0 == _filePathFromRoot.compare( 0, rit->first.size(), rit->first ) )
			{
				return (_logLevel >= rit->second);
			}
		}

		return false;
	}
};

class XsLogger
{
private:
	size_t								currentOffset_;
	boost::iostreams::mapped_file_sink	mappedFileSink_;
	size_t								mappedFileSize_;

	XsContentsLogLevelManager			xsContentsLogLevelManager_;
public:
	XsLogger::XsLogger()
		: mappedFileSize_( 1024 * 1024 * 512 )
		, currentOffset_( 0 )
	{
		boost::iostreams::mapped_file_params params;
		params.path = "filename.raw";
		params.length = mappedFileSize_;
		params.new_file_size = mappedFileSize_;
		params.flags = boost::iostreams::mapped_file::mapmode::readwrite;

		mappedFileSink_.open( params );

		if( false == mappedFileSink_.is_open() )
		{
			assert( false );
		}

		mappedFileSize_ = mappedFileSink_.size();
	}

	XsLogger::~XsLogger()
	{
		mappedFileSink_.close();
	}

	void PrintToLogFile( const char* _filePathFromRoot, const char* _callerName, int _lineNumber, XsELogLevel _logLevel, const char* fmt, ... )
	{
		if( false == xsContentsLogLevelManager_.IsPrintable( _logLevel, _filePathFromRoot ) )
		{
			return;
		}

		// 시간 정보
		boost::chrono::system_clock::time_point timePoint = boost::chrono::system_clock::now();
		std::stringstream ss;
		ss << boost::chrono::time_fmt( boost::chrono::timezone::local ) << timePoint;

		// 유저가 직접 입력한 정보
		char userInputStr[bufferSize] = { 0, };
		va_list ap;
		va_start( ap, fmt );
		vsprintf_s( userInputStr, sizeof( userInputStr ), fmt, ap );
		va_end( ap );

		// 위치 정보
		const auto msg = boost::format( "%s [%s] ***%s*** [%s:%d %s()]\r\n" ) % ss.str() % \
			xsContentsLogLevelManager_.GetLogLevelToString( _logLevel ) % userInputStr %				\
			_filePathFromRoot % _lineNumber % _callerName;

		// 파일 쓰기
		if( msg.str().size() > mappedFileSize_ - currentOffset_ )
		{
			assert( false );
		}

		memcpy_s( mappedFileSink_.data() + currentOffset_, mappedFileSize_ - currentOffset_			\
			, msg.str().c_str(), msg.str().size() );

		currentOffset_ += msg.str().size();
	}
};



XsContentsLogLevelManager xsContentsLogLevelManager_;
static XsLogger xsLogger;

#define XS_LOG_DEBUG(...) {																			\
	xsLogger.PrintToLogFile( __FILE__, __func__, __LINE__,											\
		XsELogLevel::LEVEL_CRITICAL, __VA_ARGS__);	\
}


void TestPrintLog()
{
	XS_LOG_DEBUG( "Don't divide by %d, %f %s", 1000, 0.5, "hahahahha" );
}

#undef _CRT_SECURE_NO_WARNINGS