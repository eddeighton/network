
#pragma once

#include "service/protocol/header.hpp"
#include "service/connection.hpp"

#include "vocab/service/mp.hpp"

#include <vector>
#include <unordered_map>

namespace mega::service
{
    class Router
    {
    public:
        using DirectConnections = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;
        using IndirectConnections = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;

        struct Table
        {
            // direct connections are maintained by daemon and always correct
            DirectConnections direct;
            // indirect connections are an estimate and may be incorrect
            IndirectConnections indirect;
        };

        Router( Table& table )
        :   m_table( table )
        {

        }
    private:
        Table& m_table;
    };
}

