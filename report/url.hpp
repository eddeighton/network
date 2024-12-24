
//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative works
//  based upon this software are permitted. Any copy of this software or
//  of any derivative work must include the above copyright notice, this
//  paragraph and the one after it.  Any distribution of this software or
//  derivative works must comply with all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
//  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
//  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
//  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
//  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.

#ifndef GUARD_2024_March_11_url
#define GUARD_2024_March_11_url

#include "common/serialisation.hpp"

#include <boost/url.hpp>

namespace report
{

using URL = boost::url;

URL                                      fromString( const std::string& str );
URL                                      makeFileURL( const URL& url, const boost::filesystem::path& filePath );
std::optional< std::string >             getReportType( const URL& url );
std::optional< boost::filesystem::path > getFile( const URL& url );

} // namespace report

namespace boost::serialization
{

template < class Archive >
inline void serialize( Archive& ar, report::URL& url, const unsigned int )
{
    if constexpr( Archive::is_saving::value )
    {
        std::string view = url.buffer();
        ar&         view;
    }

    if constexpr( Archive::is_loading::value )
    {
        std::string str;
        ar&         str;
        url = report::URL( str );
    }
}

} // namespace boost::serialization

#endif // GUARD_2024_March_11_url
