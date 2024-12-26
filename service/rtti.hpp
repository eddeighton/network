
#pragma once

#include <string>
#include <vector>

namespace mega::service
{

    class RTTI
    {
    public:
        using InterfaceTypeName = std::string;
        using InterfaceTypeNameVector = std::vector< InterfaceTypeName >;


    private:
        InterfaceTypeNameVector m_interfaces;
    };

}

