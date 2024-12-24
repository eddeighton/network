

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
            "/workspace/root/src/common/src/api/"s,
            "-I"s,
            "/workspace/root/src/mega/src/include/"s,
            "-I"s,
            "/build/linux_gcc_shared_debug/boost/install/include/"s,

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

}

void task_interface_analysis(TaskDependencies& dependencies)
{
    using namespace AnalysisStage;
    using namespace AnalysisStage::TestNamespace;

    Database database( dependencies.m_environment,
        dependencies.m_environment.project_manifest() );
    
    using namespace std::string_literals;

    TestObject::Args args;

    args.string = "Test String"s;
    args.array_of_string = std::vector< std::string >{};
    args.optional_string = std::optional< std::string >{};
    args.array_of_references = std::vector< TestObject* >{}; 
    args.optional_reference = std::optional< std::optional< TestObject* > >{ std::optional< TestObject* >{} };

    database.construct< TestObject >(args);

    auto fileHash = database.save_FirstFile_to_temp();

    auto pFile = boost::filesystem::createNewFileStream("/src/meta/test.cxx");
    *pFile << "// Hello World from meta pipeline\n\n";

}

}

