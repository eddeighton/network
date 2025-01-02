

// note pybind11 includes python.h and must be first include in file. 
#include <pybind11/pybind11.h>


#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>





PYBIND11_MODULE( megastructure, pythonModule )
{
    pythonModule.doc() = "Python Module for Megastructure";

    pythonModule.def(
        "test", [](){ return "Hello World from Megastructure Python Module"; }, "Test"
    );

}


