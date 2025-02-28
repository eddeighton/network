

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
#include <set>

namespace mega::meta
{

namespace
{
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace std::string_literals;

// /usr/local/clangeg/bin/clang++ -std=c++20  -I /src -fsyntax-only
// -Xclang -ast-dump  /src/test/test.interface.hpp

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
            "-Wno-pragma-once-outside-header"s,

            "-x"s,
            "c++"s,
            filePath.string(),

        };

        /// The output file associated with the command.
        cmd.Output = ""s;

        /// If this compile command was guessed rather than read from
        /// an authoritative source, a short human-readable
        /// explanation. e.g. "inferred from foo/bar.h".
        cmd.Heuristic = ""s;

        m_commands.push_back( cmd );
    }
    virtual std::vector< clang::tooling::CompileCommand >
    getCompileCommands( llvm::StringRef ) const
    {
        return m_commands;
    }
    virtual std::vector< std::string > getAllFiles() const
    {
        return m_files;
    }
    virtual std::vector< CompileCommand >
    getAllCompileCommands() const
    {
        return m_commands;
    }
};

using Namespaces = std::vector< std::string >;

Namespaces detectNamespace( const clang::DeclContext* pDeclContext )
{
    Namespaces namespaces;
    for( auto p = pDeclContext; p; p = p->getParent() )
    {
        if( auto pNamespace
            = llvm::dyn_cast< const clang::NamespaceDecl >( p ) )
        {
            namespaces.push_back( pNamespace->getNameAsString() );
        }
    }
    std::reverse( namespaces.begin(), namespaces.end() );
    return namespaces;
}

std::string fromName( const std::string& strName )
{
    std::string tmp = strName;
    boost::replace_all( tmp, "_Bool", "bool" );
    boost::replace_all( tmp, "struct ", "" );
    boost::replace_all( tmp, "class ", "" );

    VERIFY_RTE_MSG( tmp.find( '*' ) == std::string::npos,
                    "Type contains nested pointers: " << strName );
    VERIFY_RTE_MSG( tmp.find( '&' ) == std::string::npos,
                    "Type contains nested Pointer: " << strName );
    static const std::string strConst = "const"s;
    VERIFY_RTE_MSG( std::search( tmp.begin(), tmp.end(),
                                 strConst.begin(), strConst.end() )
                        == tmp.end(),
                    "Type contains nested const: " << strName );
    return tmp;
}

std::string typeNameFromFullName( const std::string& strFullTypeName,
                                  const Namespaces&  namespaces )
{
    auto i    = strFullTypeName.cbegin();
    auto iEnd = strFullTypeName.cend();
    for( const auto& strNamespace : namespaces )
    {
        VERIFY_RTE_MSG(
            strNamespace.size()
                    + std::distance( strFullTypeName.cbegin(), i ) + 2
                < strFullTypeName.size(),
            "Invalid namespace sequence for full type name: "
                << strFullTypeName );
        VERIFY_RTE_MSG(
            std::equal( i, i + strNamespace.size(),
                        strNamespace.begin(), strNamespace.end() ),
            "Mismatch between full type name and namespaces: "
                << strFullTypeName );
        i += strNamespace.size();
        i += 2;
    }
    return std::string( i, strFullTypeName.end() );
}

} // namespace

using namespace AnalysisStage;
using namespace std::string_literals;
using namespace AnalysisStage::Service;

class InterfaceCallback : public MatchFinder::MatchCallback
{
    Database&               database;
    boost::filesystem::path interfaceFilePath;

public:
    InterfaceCallback( Database&               _database,
                       boost::filesystem::path interfaceFilePath )
        : database( _database )
        , interfaceFilePath( interfaceFilePath )
    {
    }
    virtual void run( const MatchFinder::MatchResult& Result )
    {
        auto toQualifiedType =
            [ this ]( const clang::QualType& type ) -> QualifiedType*
        {
            if( type->isPointerType() )
            {
                auto tmp = type->getPointeeType();
                if( tmp.isLocalConstQualified() )
                {
                    tmp.removeLocalConst();
                    VERIFY_RTE_MSG( !tmp.isLocalConstQualified(),
                                    "Failed to remove const" );
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( tmp.getAsString() ) },
                            true,  // is_const
                            true,  // is_pointer
                            false, // is_reference
                            false  // is_rvalue
                        } );
                }
                else
                {
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( tmp.getAsString() ) },
                            false, // is_const
                            true,  // is_pointer
                            false, // is_reference
                            false  // is_rvalue
                        } );
                }
            }
            else if( type->isReferenceType() )
            {
                auto tmp = type.getNonReferenceType();
                if( tmp.isConstQualified() )
                {
                    tmp.removeLocalConst();
                    VERIFY_RTE_MSG( !tmp.isLocalConstQualified(),
                                    "Failed to remove const" );
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( tmp.getAsString() ) },
                            true,  // is_const
                            false, // is_pointer
                            true,  // is_reference
                            false  // is_rvalue
                        } );
                }
                else
                {
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( tmp.getAsString() ) },
                            false, // is_const
                            false, // is_pointer
                            true,  // is_reference
                            false  // is_rvalue
                        } );
                }
            }
            else
            {
                if( type.isLocalConstQualified() )
                {
                    auto tmp = type;
                    tmp.removeLocalConst();
                    VERIFY_RTE_MSG( !tmp.isLocalConstQualified(),
                                    "Failed to remove const" );
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( tmp.getAsString() ) },
                            true,  // is_const
                            false, // is_pointer
                            false, // is_reference
                            false  // is_rvalue
                        } );
                }
                else
                {
                    return database.construct< QualifiedType >(
                        QualifiedType::Args{
                            Service::Type::Args{
                                fromName( type.getAsString() ) },
                            false, // is_const
                            false, // is_pointer
                            false, // is_reference
                            false  // is_rvalue
                        } );
                }
            }
        };

        if( auto pRecordDecl
            = Result.Nodes.getNodeAs< clang::RecordDecl >( "interface"
                                                           "s" ) )
        {
            auto namespaces
                = detectNamespace( pRecordDecl->getDeclContext() );

            if( !namespaces.empty() && namespaces.front() == "mega" )
            {
                // determine if interface or factory via inheritance
                bool bIsInterface = false;
                bool bIsFactory   = false;
                if( const auto* pCXXRecordDecl
                    = dyn_cast< CXXRecordDecl >( pRecordDecl ) )
                {
                    auto type = pRecordDecl->getASTContext()
                                    .getTypeDeclType( pRecordDecl );
                    const std::string strFullTypeName
                        = fromName( type.getAsString() );
                    if( pCXXRecordDecl->hasDefinition() )
                    {
                        // ignore the virtual base classes themselves
                        using namespace std::string_literals;
                        static const std::set< std::string >
                            baseClassTypes
                            = { "mega::service::Interface"s,
                                "mega::service::Factory"s };
                        if( baseClassTypes.contains(
                                strFullTypeName ) )
                        {
                            return;
                        }

                        for( const auto pBase :
                             pCXXRecordDecl->bases() )
                        {
                            const std::string strType
                                = pBase.getType()
                                      .getCanonicalType()
                                      .getAsString();
                            if( strType
                                == "struct mega::service::Interface" )
                            {
                                bIsInterface = true;
                            }
                            else if(
                                strType
                                == "struct mega::service::Factory" )
                            {
                                bIsInterface = true;
                                bIsFactory   = true;
                            }
                        }
                    }
                }

                if( bIsInterface )
                {
                    auto type = pRecordDecl->getASTContext()
                                    .getTypeDeclType( pRecordDecl );
                    const std::string strFullTypeName
                        = fromName( type.getAsString() );
                    // std::cout << "Found RecordDecl: " <<
                    // type.getAsString()  << std::endl;
                    const std::string strTypeName
                        = typeNameFromFullName(
                            strFullTypeName, namespaces );

                    Interface* pInterface
                        = database.construct< Interface >(
                            Interface::Args{
                                interfaceFilePath.string(),
                                strTypeName,
                                strFullTypeName,
                                namespaces,
                                {} } );

                    if( bIsFactory )
                    {
                        database.construct< Factory >(
                            Factory::Args{ pInterface } );
                    }

                    // determine functions
                    for( const auto* pNamingChild :
                         pRecordDecl->decls() )
                    {
                        if( const auto* pFunctionDecl
                            = llvm::dyn_cast< FunctionDecl >(
                                pNamingChild ) )
                        {
                            if( pFunctionDecl->isUserProvided() )
                            {
                                // std::cout << "Found interface
                                // function: " <<
                                // pFunctionDecl->getNameAsString()
                                //     << " with arguments: " <<
                                //     pFunctionDecl->getNumParams()
                                //     << std::endl;

                                std::vector< Parameter* > parameters;
                                for( auto i = 0U;
                                     i
                                     != pFunctionDecl->getNumParams();
                                     ++i )
                                {
                                    auto pParam
                                        = pFunctionDecl->getParamDecl(
                                            i );
                                    auto pQualifiedType
                                        = toQualifiedType(
                                            pParam->getType()
                                                .getCanonicalType() );
                                    parameters.push_back(
                                        database.construct<
                                            Parameter >( { Parameter::Args{
                                            pQualifiedType,
                                            pParam
                                                ->getNameAsString() } } ) );
                                }

                                auto pReturnType = toQualifiedType(
                                    pFunctionDecl->getReturnType()
                                        .getCanonicalType() );
                                auto pFunction = database.construct<
                                    Function >( Function::Args{
                                    pFunctionDecl->getNameAsString(),
                                    parameters, pReturnType } );
                                pInterface->push_back_functions(
                                    pFunction );
                            }
                        }
                    }
                }
            }
        }
    }
};

void task_interface_analysis( TaskDependencies& dependencies )
{
    const mega::io::CompilationFilePath compilationFilePath
        = dependencies.m_environment.AnalysisStage_AnalysisFile(
            dependencies.m_environment.project_manifest() );

    TASK_START( "task_interface_analysis" );

    // has all input source files
    // NOTE: not worrying about include directives here
    task::DeterminantHash determinant(
        dependencies.m_configuration.pipelineHash,
        dependencies.m_configuration.interfacePaths );

    if( dependencies.m_environment.restore(
            compilationFilePath, determinant ) )
    {
        dependencies.m_environment.setBuildHashCode(
            compilationFilePath );
        TASK_CACHED( "task_interface_analysis" );
        return;
    }

    Database database(
        dependencies.m_environment,
        dependencies.m_environment.project_manifest() );

    for( const auto& interfacePath :
         dependencies.m_configuration.interfacePaths )
    {
        VERIFY_RTE( boost::filesystem::exists( interfacePath ) );
        TASK_PROGRESS( "task_interface_analysis",
                       "Processing: " << interfacePath.string() );

        ToolDB      db( interfacePath );
        ClangTool   tool( db, { interfacePath.string() } );
        MatchFinder finder;

        InterfaceCallback interfaceCallback(
            database, interfacePath );
        DeclarationMatcher interfaceMatcher
            = recordDecl().bind( "interfaces" );
        finder.addMatcher( interfaceMatcher, &interfaceCallback );

        tool.run( newFrontendActionFactory( &finder ).get() );
    }

    using namespace AnalysisStage::Service;

    auto compilationFileHash = database.save_AnalysisFile_to_temp();

    dependencies.m_environment.temp_to_real( compilationFilePath );
    dependencies.m_environment.setBuildHashCode(
        compilationFilePath, compilationFileHash );
    dependencies.m_environment.stash(
        compilationFilePath, determinant );

    TASK_COMPLETE( "task_interface_analysis" );
}

} // namespace mega::meta
