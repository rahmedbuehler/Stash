#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread

class Stash_Server
{
    private:
        int m_port;
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

            // Check if pull/push call makes sense
            if (args.size() < 1 or (args[0] != "push" and args[0] != "pull"))
            {
                args[0] = "Invalid Operation";
            }
            return args;
        }

        void send(socket, std::string message)
        {
            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }

        void push_files()
        {
        }

        void pull_files()
        {
        }

    public:
        Stash_Server(boost::asio::io_context& io_context, int port = 3490)
            // Create acceptor for any ipv4 endpoint; opens, binds, and listens on endpoint
            : m_port{port}, m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        {
            m_files_stored = false;
        }

        void run()
        {
            for(;;)
            {
                try
                {
                    boost::asio::ip::tcp::socket server_socket(io_context);
                    m_acceptor.accept(server_socket); // Accept connection to socket

                    std::vector <char> data (128);
                    boost::asio::read(server_socket, data)
                    std::vector <std::string> args {parse_first_request(data)};

                    switch(args[0])
                    {
                        case "push":
                            if (m_files_stored)
                            {
                                boost::asio::write(server_socket, boost::asio::buffer("!"));
                            }
                            push_files(server_socket);
                            break;
                        case "pull":
                            if (not m_files_stored)
                            {
                                boost::asio::write(server_socket, boost::asio::buffer("!"));
                                break;
                            }
                            pull_files(server_socket);
                            break;
                        default:
                            break;
                    }

                    // Close <server_socket>
                    server_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    server_socket.close();

                }
                catch (boost::system::system_error &e)
                {
                    std::cerr << "\tError in " << e.what() << std::endl;
                }
            }
        }
};

int main()
{
  try
  {
    boost::asio::io_context io_context;
    Stash_Server server (io_context);
    server.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

