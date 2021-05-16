#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread

class Stash_Server
{
    private:
        int m_port;
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::acceptor m_acceptor;
        bool m_files_stored;
        std::vector <std::vector <std::byte>> m_storage;

    public:
        Stash_Server(int port = 3490)
            : m_port{port}, m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port)) // Create acceptor for any ipv4
        {
            m_files_stored = false;
        }

        void send(socket, std::string message)
        {
            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }

        void run()
        {
            boost::asio::ip::tcp::socket server_socket(m_io_context);

            for(;;)
            {
                m_acceptor.accept(server_socket); // Accept connection to socket

                std::vector <char> data (1);
                boost::asio::read(server_socket, data)

                // Check if pull/push call makes sense
                if ((data[0] == "o" and m_files_stored) or (data[0]= "i" and not m_files_stored))
                    boost::asio::write(server_socket, boost::asio::buffer("!"));
                else
                    boost::asio::write(server_socket, boost::asio::buffer(" "));
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

