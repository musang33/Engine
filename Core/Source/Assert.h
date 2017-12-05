#pragma once

#include <boost/test/unit_test.hpp>
#include <boost/stacktrace.hpp>
#include <boost/format.hpp>

namespace SSL
{
	#define SSL_ASSERT( P, E )																		\
	if( !( P ) )																					\
	{																								\
		const boost::test_tools::assertion_result ar = BOOST_TEST_BUILD_ASSERTION( P ).evaluate();	\
		const boost::unit_test::const_string cstrPred = ar.message();								\
		const std::string strPred( cstrPred.begin(), cstrPred.end() );								\
		const boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace();					\
		const auto msg = boost::format(	"Assertion failed : [%s]%s at %s:%d:%s()\n%s" ) %			\
			BOOST_STRINGIZE( P ) % strPred % __FILE__ % __LINE__ % __func__ %						\
			boost::stacktrace::detail::to_string( &st.as_vector()[ 0 ], st.size() );				\
		throw E( msg.str() );																		\
	}
}
