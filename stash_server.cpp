#include <iostream>
#include <string>
#include <boost/asio.hpp>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread

class Stash_Server
{
    private:
        int m_port;
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::acceptor m_acceptor;

    public:
        Stash_Server(int port = 3490)
            : m_port{port}, m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port)) // Create acceptor for any ipv4
        {
        }

        void send(socket, std::string message)
        {
            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }

        void run()
        {
            boost::asio::ip::tcp::socket socket(m_io_context);

            for(;;)
            {
                m_acceptor.accept(socket); // Accept connection to socket
                send(socket, "Hello there");
            }
        }
};

int main()
{
  try
  {
    Stash_Server server;
    server.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

