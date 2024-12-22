//  Copyright (c) Deighton Systems Limited. 2025. All Rights Reserved.
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


#include "meta/configuration.hpp"
#include "meta/environment.hpp"

#include "pipeline/pipeline.hpp"
#include "pipeline/task.hpp"

#include "common/serialisation.hpp"

namespace mega::meta
{

namespace
{
// meta pipeline concrete Task
struct Task
{
    std::string strTaskName;
};


pipeline::TaskDescriptor encode( const Task& task )
{
    //std::ostringstream os;
    //{
    //    boost::archive::binary_oarchive oa( os );
    //    oa&                             task;
    // }
    return { task.strTaskName, "meta", "" };
}

Task decode( const pipeline::TaskDescriptor& taskDescriptor )
{
    Task task{ taskDescriptor.getName() };
    //{
    //    std::istringstream              is( taskDescriptor.getBuffer() );
    //    boost::archive::binary_iarchive ia( is );
    //    ia&                             task;
    // }
    return task;
}

class MetaPipeline : public pipeline::Pipeline
{
    struct Config
    {
        meta::Configuration       m_configuration;
        mega::io::Manifest        m_manifest;
        mega::io::MetaEnvironment m_environment;

        Config( const pipeline::Configuration& pipelineConfig )
            : m_configuration( fromPipelineConfiguration( pipelineConfig ) )
            , m_manifest( m_configuration.manifestData )
            , m_environment( m_configuration.directories )
        {
        }
    };
    std::unique_ptr< Config > m_pConfig;
    
    // pipeline::Pipeline
    virtual pipeline::Schedule getSchedule( pipeline::Progress& progress, pipeline::Stash& stash ) override
    {
        VERIFY_RTE( m_pConfig );

        using TskDesc    = pipeline::TaskDescriptor;
        using TskDescVec = pipeline::TaskDescriptor::Vector;

        Configuration& config = m_pConfig->m_configuration;

        pipeline::Dependencies dependencies;

        const TskDesc generateComponents = encode( Task{ "Testing" } );
        dependencies.add( generateComponents, {} );

        return { dependencies };
    }

    virtual void execute( const pipeline::TaskDescriptor& pipelineTask, pipeline::Progress& progress,
                                        pipeline::Stash& stash, pipeline::DependencyProvider& dependencies ) override
    {
        VERIFY_RTE( m_pConfig );

        Configuration& config = m_pConfig->m_configuration;

        const Task task = decode( pipelineTask );

        mega::io::MetaEnvironment environment( config.directories );

    }

    virtual void initialise( const pipeline::Configuration& pipelineConfig, std::ostream& osLog ) override
    {
        m_pConfig = std::make_unique< Config >( pipelineConfig );
        osLog << "SUCCESS: Initialised Meta Pipeline: " << m_pConfig->m_configuration.header.pipelineID
              << " with version: " << m_pConfig->m_configuration.header.version << "\n";
    }

};

} // namespace

extern "C" BOOST_SYMBOL_EXPORT MetaPipeline mega_pipeline;
MetaPipeline                                mega_pipeline;

}

