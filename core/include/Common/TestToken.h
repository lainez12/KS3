#pragma once

namespace Kub3::Tools::Tester
{
    class ProcedureTestController; // Forward declaration of the authorized class
}

namespace Kub3
{
    class TestToken
    {
    private:
        // Only authorized classes can create this token
        friend class Kub3::Tools::Tester::ProcedureTestController;

        TestToken() = default; // Private constructor
    };
}
