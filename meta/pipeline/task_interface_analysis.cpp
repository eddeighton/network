

#include "common/assert_verify.hpp"
#include "common/file.hpp"

////////////////////////////////////////////
#include "common/clang_warnings_begin.hpp"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTConsumer.h"

#include "clang/Basic/Thunk.h"
#include "clang/AST/Mangle.h"

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"

#include "common/clang_warnings_end.hpp"
////////////////////////////////////////////

#include "meta/db/database/AnalysisStage.hxx"
#include "meta/environment.hpp"
#include "meta/task.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace mega::meta
{

namespace
{
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace std::string_literals;

struct ToolDB : public clang::tooling::CompilationDatabase
{
    std::vector< clang::tooling::CompileCommand > m_commands;
    std::vector< std::string >                    m_files;

    ToolDB( const boost::filesystem::path& filePath )
    {
        m_files.push_back( filePath.string() );

        clang::tooling::CompileCommand cmd;

        cmd.Directory = filePath.parent_path().string();

        /// The source file associated with the command.
        cmd.Filename = filePath.filename().string();

        /// The command line that was executed.
        cmd.CommandLine = std::vector< std::string >{

            "/usr/local/clangeg/bin/clang-15"s,

            "-resource-dir"s,
            "/usr/local/clangeg/lib/clang/15.0.0"s,

            "-I"s,
            "/src"s,

            "-std=c++20"s,
            "-fcoroutines-ts"s,
            "-fgnuc-version=4.2.1"s,
            "-fno-implicit-modules"s,
            "-fcxx-exceptions"s,
            "-fexceptions"s,
            "-fcolor-diagnostics"s,
            "-faddrsig"s,
            "-D__GCC_HAVE_DWARF2_CFI_ASM=1"s,

            "-Wno-unused-command-line-argument"s,

            "-x"s,
            "c++"s,
            filePath.string(),

        };

        /// The output file associated with the command.
        cmd.Output = ""s;

        /// If this compile command was guessed rather than read from an authoritative
        /// source, a short human-readable explanation.
        /// e.g. "inferred from foo/bar.h".
        cmd.Heuristic = ""s;

        m_commands.push_back( cmd );
    }
    virtual std::vector< clang::tooling::CompileCommand > getCompileCommands( llvm::StringRef ) const
    {
        return m_commands;
    }
    virtual std::vector< std::string >    getAllFiles() const { return m_files; }
    virtual std::vector< CompileCommand > getAllCompileCommands() const { return m_commands; }
};


using Namespaces = std::vector< std::string >;

Namespaces detectNamespace( const clang::DeclContext* pDeclContext )
{
    Namespaces namespaces;
    for( auto p = pDeclContext; p; p = p->getParent() )
    {
        if( auto pNamespace = llvm::dyn_cast< const clang::NamespaceDecl >( p ) )
        {
            namespaces.push_back( pNamespace->getNameAsString() );
        }
    }
    std::reverse( namespaces.begin(), namespaces.end() );
    return namespaces;
}
}


using namespace AnalysisStage;

void task_interface_analysis(TaskDependencies& dependencies)
{
    class InterfaceCallback : public MatchFinder::MatchCallback
    {
        Database& database;

        public:
        InterfaceCallback( Database& _database )
            : database( _database )
        {
        }
        virtual void run( const MatchFinder::MatchResult& Result )
        {
           if( auto pRecordDecl = Result.Nodes.getNodeAs< clang::RecordDecl >( "interfaces" ) )
           {
               auto namespaces = detectNamespace( pRecordDecl->getDeclContext() );

                if( !namespaces.empty() && namespaces.front() == "mega" )
                {
                    // ignore service base interfaces
                    if( ( namespaces.size() == 2 ) && ( namespaces.back() == "service" ) )
                    {
                        return;
                    }

                    auto type = pRecordDecl->getASTContext().getTypeDeclType( pRecordDecl );
                    std::cout << "Found RecordDecl: " << type.getAsString()  << std::endl;
                    
                    
                        
                    
                    
           //         try
           //         {
           //             Type recordType{ Mutable{ fromName( type.getAsString() ) } };
           //             auto iFind = std::find( model.inlineTypes.begin(), model.inlineTypes.end(), recordType );
           //             if( iFind == model.inlineTypes.end() )
           //             {
           //                 model.inlineTypes.push_back( recordType );
           //             }
           //         }
           //         catch( std::exception& ex )
           //         {
           //             THROW_RTE( "Fail to analyse function type for: " << pRecordDecl->getNameAsString() << " with type: "
           //                     << type.getAsString() << " error: " << ex.what() );
           //         }
                }
           }
        }
    };

    Database database( dependencies.m_environment,
        dependencies.m_environment.project_manifest() );

    for( const auto& interfacePath : dependencies.m_configuration.interfacePaths)
    {
        VERIFY_RTE( boost::filesystem::exists(interfacePath) );
        std::cout << "Got interface path: " << interfacePath.string() << std::endl;

        ToolDB      db( interfacePath );
        ClangTool   tool( db, { interfacePath.string() } );
        MatchFinder finder;

        InterfaceCallback interfaceCallback( database );
        DeclarationMatcher interfaceMatcher = recordDecl().bind( "interfaces" );
        finder.addMatcher( interfaceMatcher, &interfaceCallback );

        tool.run( newFrontendActionFactory( &finder ).get() );

    }

    auto pFile = boost::filesystem::createNewFileStream("/src/test/service/test.cxx");
    *pFile << "// Hello World from meta pipeline\n\n";

    using namespace AnalysisStage::TestNamespace;

    
    using namespace std::string_literals;

    // TestObject::Args args;

    // args.string = "Test String"s;
    // args.array_of_string = std::vector< std::string >{};
    // args.optional_string = std::optional< std::string >{};
    // args.array_of_references = std::vector< TestObject* >{}; 
    // args.optional_reference = std::optional< std::optional< TestObject* > >{ std::optional< TestObject* >{} };

    // database.construct< TestObject >(args);

    // auto fileHash = database.save_FirstFile_to_temp();

}

}

