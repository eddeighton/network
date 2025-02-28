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

#ifndef STASH_20_MAY_2022
#define STASH_20_MAY_2022

#include "vocab/compilation/symbol_table.hpp"

#include "common/stash.hpp"

namespace mega::pipeline
{
class Stash
{
public:
    virtual ~Stash();

    virtual task::FileHash    getBuildHashCode( const boost::filesystem::path& filePath )                          = 0;
    virtual void              setBuildHashCode( const boost::filesystem::path& filePath, task::FileHash hashCode ) = 0;
    virtual void              stash( const boost::filesystem::path& file, task::DeterminantHash code )             = 0;
    virtual bool              restore( const boost::filesystem::path& file, task::DeterminantHash code )           = 0;
    virtual mega::SymbolTable getSymbolTable()                                                                     = 0;
    virtual mega::SymbolTable newSymbols( const mega::SymbolRequest& request )                                     = 0;
};
} // namespace mega::pipeline

#endif // STASH_20_MAY_2022
