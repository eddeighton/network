

// note pybind11 includes python.h and must be first include in file. 
#include <pybind11/pybind11.h>

#include "service/interface/test.interface.hpp"


#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <iostream>

namespace mega::service
{
    // Ptr< Connectivity > getConnectivity()
    // {
    //     
    // }
}

PYBIND11_MODULE( megastructure, pythonModule )
{
    pythonModule.doc() = "Python Module for Megastructure";

    pythonModule.def(
        "test", [](){ return "Hello World from Megastructure Python Module"; }, "Test"
    );

    // pythonModule.def(
    //     "Connectivity", mega::service::getConnectivity, "Get Connectivity"
    // )

    // pybind11::class_< mega::service::Connectivity, mega::service::Ptr< Connectivity > >( pythonModule, "Connectivity" )
    //     .def( "shutdown", &Connectivity::shutdown )
    //     ;

}


