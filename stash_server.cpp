#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread

class Stash_Session
{
    private:
        boost::asio::ip::tcp::socket m_session_sock;
        std::vector <std::vector <std::byte>> m_storage;

        bool files_stored()
        {
            if (m_storage.size() == 0)
                return false;
            else
                return true;
        }

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
            m_storage.clear();
        }

        void pull_files()
        {
        }

    public:
        Stash_Session(boost::asio::ip::tcp::socket& session_sock, std::vector <std::vector <std::byte>>& storage)
        {
            // This vs passing pointer?
            m_session_sock = session_sock;
            m_storage = storage;
        }

        void start()
        {
            std::vector <char> data (128);
            boost::asio::read(m_session_sock, data)
            std::vector <std::string> args {parse_first_request(data)};

            switch(args[0])
            {
                case "push":
                    if (files_stored())
                    {
                        boost::asio::write(m_session_sock, boost::asio::buffer("!"));
                    }
                    push_files(m_session_sock);
                    break;
                case "pull":
                    if (not files_stored())
                    {
                        boost::asio::write(m_session_sock, boost::asio::buffer("!"));
                        break;
                    }
                    pull_files(m_session_sock);
                    break;
                default:
                    break;
            }

        }



class Stash_Server
{
    private:
        int m_port;
        // This eventually needs to be generalized for more than one user
        std::vector <std::vector <std::byte>> m_storage;
        boost::asio::ip::tcp::acceptor m_acceptor;


    public:
        Stash_Server(boost::asio::io_context& io_context, int port = 3490)
            // Create acceptor for any ipv4 endpoint; opens, binds, and listens on endpoint
            : m_port{port}, m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        {
        }

        void run()
        {
            for(;;)
            {
                try
                {

                    // Create and accept connection to socket
                    boost::asio::ip::tcp::socket session_socket(io_context);
                    m_acceptor.accept(session_socket);


                    // Handle current session
                    Stash_Session session (session_socket, m_storage);
                    session.run();

                    // Close socket
                    session_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    session_socket.close();
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

