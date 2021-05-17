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

        std::vector <std::string> parse_first_request (std::vector<char> data)
        {
            std::string current_arg {""};
            std::vector <std::string> args;
            for (int i{0}; i < data.size(); i++)
            {
                if (data[i] == ' ' or i == data.size()-1)
                {
                    args.push_back(current_arg);
                    current_arg = "";
                }
                else
                    current_arg += data[i];
            }

            return args;
        }



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

                std::vector <char> data (128);
                boost::asio::read(server_socket, data)
                std::vector <std::string> args {parse_first_request(data)};

                // Check if pull/push call makes sense
                if (args.size() < 1 or (args[0] != "push" and args[0] != "pull")
                {
                    boost::asio::write(server_socket, boost::asio::buffer("Error"));
                    continue;
                }
                else if ((args[0] == "push" and m_files_stored) or (args[0]= "pull" and not m_files_stored))
                    boost::asio::write(server_socket, boost::asio::buffer("!"));
                else
                    boost::asio::write(server_socket, boost::asio::buffer(" "));

                // Main Transfer

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

