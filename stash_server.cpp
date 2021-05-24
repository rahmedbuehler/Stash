#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread


// Add smart pointers

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

            // Check if call makes sense
            if (args.size() < 1)
                args[0] = "Invalid Operation";
            else if (args[0] == "push" and args.size() > 1)
                args[1] = std::stoi(args[1]);
            else if (args[0] == "pull")
                ;
            else
                args[0] = "Invalid Operation";

            return args;
        }

        std::size_t send(const std::string & message)
        {
            return boost::asio::write(m_session_sock, boost::asio::buffer(message));
        }

        std::size_t send(const std::vector <std::byte> & file)
        {
            return boost::asio::write(m_session_sock, boost::asio::buffer(file));
        }

        void receive_files(const std::size_t num_files)
        {
            for (std::size_t i{0}; i < num_files; i++)
            {
                std::vector <std::byte> current_file;
                boost::asio::read(m_session_sock, current_file);
                m_storage.push_back(current_file);
            }
        }

        void send_files()
        {
            send(std::to_string(m_storage.size()));

            for (auto current_file : m_storage)
            {
                send(current_file);
            }

            m_storage.clear();
        }

    public:
        Stash_Session(boost::asio::ip::tcp::socket & session_sock, std::vector <std::vector <std::byte>> & storage)
        {
            m_session_sock = session_sock;
            m_storage = storage;
        }

        void start()
        {
            std::vector <char> data (128);
            boost::asio::read(m_session_sock, data);
            std::vector <std::string> args = parse_first_request(data);

            if (args[0] == "push")
                receive_files(args[1]);
            else if (args[0] == "pull")
                send_files();
        }
};


class Stash_Server
{
    private:
        int m_port;
        // This eventually needs to be generalized for more than one user
        std::vector <std::vector <std::byte>> m_storage;
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::acceptor m_acceptor;


    public:
        Stash_Server(boost::asio::io_context& io_context, int port = 3490)
            // Create acceptor for any ipv4 endpoint; opens, binds, and listens on endpoint
            : m_port{port}, m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        {
            m_io_context = io_context;
        }

        void run()
        {
            for(;;)
            {
                    // Create and accept connection to socket
                    boost::asio::ip::tcp::socket session_socket(m_io_context);
                    m_acceptor.accept(session_socket);

                    // Handle current session
                    Stash_Session session (session_socket, m_storage);
                    session.start();

                    // Close socket
                    session_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    session_socket.close();
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
