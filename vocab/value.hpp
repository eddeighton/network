
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

#ifndef GUARD_2023_October_17_value
#define GUARD_2023_October_17_value

// check build system
// #define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
// #define BOOST_MPL_LIMIT_LIST_SIZE 30
// #include <boost/mpl/list.hpp>
// #undef BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

// #include "database/directories.hpp"
// #include "database/manifest_data.hpp"
// #include "database/component_info.hpp"

// #include "vocab/compilation/interface/object_id.hpp"
// #include "vocab/compilation/interface/relation_id.hpp"
// #include "vocab/compilation/interface/sub_object_id.hpp"
// #include "vocab/compilation/interface/symbol_id.hpp"
// #include "vocab/compilation/interface/type_id.hpp"
// 
// #include "vocab/compilation/concrete/instance.hpp"
// #include "vocab/compilation/concrete/object_id.hpp"
// #include "vocab/compilation/concrete/sub_object_id_instance.hpp"
// #include "vocab/compilation/concrete/sub_object_id.hpp"
// #include "vocab/compilation/concrete/type_id_instance.hpp"
// #include "vocab/compilation/concrete/type_id.hpp"

// #include "vocab/compilation/compilation_configuration.hpp"
// #include "vocab/compilation/icontext_flags.hpp"
// #include "vocab/compilation/invocation_id.hpp"
// #include "vocab/compilation/megastructure_installation.hpp"
// #include "vocab/compilation/size_alignment.hpp"
// #include "vocab/compilation/type_id_sequence.hpp"

// #include "vocab/runtime/pointer.hpp"
// #include "vocab/runtime/timestamp.hpp"

#include "vocab/service/mp.hpp"
#include "vocab/service/mpo.hpp"
#include "vocab/service/mptf.hpp"
// #include "vocab/service/logical_thread_id.hpp"
// #include "vocab/service/node.hpp"
// #include "vocab/service/program.hpp"
// #include "vocab/service/project.hpp"
// #include "vocab/service/root_config.hpp"

#include <boost/filesystem/path.hpp>

#include <variant>
#include <string>

namespace mega
{

using Value = std::variant<

#define MEGA_VALUE_TYPE( TypeName ) TypeName,
#include "vocab/value.hxx"
#undef MEGA_VALUE_TYPE

    std::string, boost::filesystem::path >;

} // namespace mega

#endif // GUARD_2023_October_17_value
