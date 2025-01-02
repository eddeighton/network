

// note pybind11 includes python.h and must be first include in file. 
#include <pybind11/pybind11.h>

#include "service/interface/test.interface.hpp"
#include "service/interface/connectivity.interface.hpp"

#include "service/connect.hpp"
#include "service/ptr.hpp"
#include "vocab/service/mp.hpp"

#include "common/serialisation.hpp"

#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <boost/archive/xml_oarchive.hpp>

#include <iostream>
#include <string>
#include <memory>
#include <sstream>

using namespace std::string_literals;

namespace mega::service
{

    class Connection
    {
    public:
        std::unique_ptr< mega::service::Connect > m_pConnection;

        Connection( std::string strIPAddress = "localhost"s,
                short portNumber = 1234 )
            :   m_pConnection( std::make_unique< mega::service::Connect >(
                    mega::service::IPAddress{ strIPAddress },
                    mega::service::PortNumber{ portNumber } ) )
        {
        }

        static Connection createDefault() { return Connection(); }

        MP getMP() const
        {
            return m_pConnection->getMP();
        }

        MP getDaemonMP() const
        {
            return m_pConnection->getDaemonMP();
        }

        std::string printRegistration()
        {
            const auto registration = 
                mega::service::Registry::getReadAccess()->getRegistration();

            std::ostringstream os;
            {
                boost::archive::xml_oarchive xml( os );
                xml & boost::serialization::make_nvp( "registration", registration );
            }

            return os.str();
        }

        template< typename T >
        Ptr< T > get( MP mp )
        {
            auto reg = mega::service::Registry::getReadAccess();
            return reg->one< T >( mp );
        }
    };
}

PYBIND11_MODULE( megastructure, pythonModule )
{
    pythonModule.doc() = "Python Module for Megastructure";

    pybind11::class_< mega::service::MP >( pythonModule, "MP" )
        .def
        ( 
            pybind11::init
            ( 
                [](int m, int p) -> mega::service::MP
                {
                    using namespace mega::service;
                    return MP
                    (
                        MachineID(static_cast<MachineID::ValueType>(m)),
                        ProcessID(static_cast<ProcessID::ValueType>(p))
                    );
                }
            ) 
        )
        .def( "MachineID", [](const mega::service::MP& mp) { return mp.getMachineID().getValue(); } )
        .def( "ProcessID", [](const mega::service::MP& mp) { return mp.getProcessID().getValue(); } )
        .def( "valid", &mega::service::MP::valid )
        .def( "__repr__", [](const mega::service::MP& mp) { std::ostringstream os; os << mp; return os.str(); })
        .def( "__str__", [](const mega::service::MP& mp) { std::ostringstream os; os << mp; return os.str(); })
        ;

    pybind11::class_< mega::service::Ptr<mega::service::Connectivity> >( pythonModule, "Connectivity" )
        .def( "shutdown", [](mega::service::Ptr<mega::service::Connectivity> p){ p->shutdown(); } )
        ;

    pybind11::class_< mega::service::Connection >( pythonModule, "Connection" )
        .def( pybind11::init< std::string, short >() )
        .def( pybind11::init( &mega::service::Connection::createDefault ))
        .def( "mp", &mega::service::Connection::getMP )
        .def( "daemon", &mega::service::Connection::getDaemonMP )
        .def( "__repr__", &mega::service::Connection::printRegistration )
        .def( "__str__", &mega::service::Connection::printRegistration )
        .def( "Connectivity", &mega::service::Connection::get< mega::service::Connectivity > )
        ;

}


