
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
        using DirectConnections = std::unordered_map< MP, Connection::WeakPtr >;
        using IndirectConnections = std::unordered_map< MP, Connection::WeakPtr >;
        using Connections = std::vector< Connection::WeakPtr >;

        struct Table
        {
            DirectConnections direct;
            IndirectConnections indirect;
            Connections connections;
        };

        Router( Table& table )
        :   m_table( table )
        {

        }
    private:
        Table& m_table;
    };
}

