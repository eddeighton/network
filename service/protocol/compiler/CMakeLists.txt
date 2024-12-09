

add_executable( protocol_compiler
   generator.hpp
   generator.cpp
   grammar.hpp
   grammar.cpp
   json_converter.hpp
   json_converter.cpp
   main.cpp
)
target_include_directories( protocol_compiler PRIVATE ${SRC_ROOT_DIR})
target_link_libraries( protocol_compiler Boost::program_options )
target_link_libraries( protocol_compiler Boost::fiber )
target_link_libraries( protocol_compiler pantor::inja )
target_link_libraries( protocol_compiler nlohmann_json::nlohmann_json )
target_link_libraries( protocol_compiler Common::commonlib ) 

