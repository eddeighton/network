
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

#ifndef GUARD_2023_sizealign_21_oct
#define GUARD_2023_sizealign_21_oct

#include "vocab/native_types.hpp"

#ifndef MEGAJIT
#include <ostream>
#endif

namespace mega
{
struct SizeAlignment
{
    U64 size      = 0U;
    U64 alignment = 0U;

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& size;
        archive& alignment;
    }
};
#ifndef MEGAJIT
inline std::ostream& operator<<( std::ostream& os, const mega::SizeAlignment& sizeAlignment )
{
    return os << "( size: " << sizeAlignment.size << " align: " << sizeAlignment.alignment << " )";
}
#endif
}

#endif //GUARD_2023_sizealign_21_oct