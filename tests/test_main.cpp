#include <QCoreApplication>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Catch::Session session;

    return session.run(argc, argv);
}
