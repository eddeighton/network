




set( JINJA_TEMPLATES 
	templates/branch.jinja
	templates/graph.jinja
	templates/multiline.jinja
	templates/report.jinja
	templates/plot.jinja
	templates/table.jinja
	)



set( REPORTS_HEADERS
    colours.hxx
    html_template_engine.hpp
    key_code.hpp
    renderer_html.hpp
    renderer_html.tpp
    report.hpp
    reporter_id.hpp
    url.hpp
    value.hpp
)

set( REPORTS_SOURCE
    colours.cxx
    html_template_engine.cpp
    url.cpp
)

add_library( reportlib ${REPORTS_HEADERS} ${REPORTS_SOURCE} )
add_library( Report::reportlib ALIAS reportlib )

target_link_libraries( reportlib Boost::filesystem )
target_link_libraries( reportlib Boost::iostreams )
target_link_libraries( reportlib Boost::serialization )
target_link_libraries( reportlib Boost::url )
target_link_libraries( reportlib pantor::inja )
target_link_libraries( reportlib nlohmann_json::nlohmann_json )
target_link_libraries( reportlib Common::commonlib ) 

add_subdirectory(test)
