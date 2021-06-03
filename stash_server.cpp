#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread


// Add smart pointers

class Stash_Session
{
    private:
        boost::asio::ip::tcp::socket * m_session_sock_ptr;
        std::vector <std::vector <std::byte>> * m_storage_ptr;

        bool files_stored()
        {
            if (m_storage_ptr->size() == 0)
                return false;
            else
                return true;
        }

        std::vector <std::string> parse_first_request (const std::vector<char> & data)
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
                {
                    current_arg += data[i];
                }
            }

            std::cout << "Identified " << args.size() << " argument(s) in parse_first_request\n";
            for (std::string arg : args)
                std::cout << "\t" << arg <<"\n";

            // Check if call makes sense
            if (args.size() < 1)
                args[0] = "Invalid Operation";
            else if (args[0] == "push" and args.size() > 1)
                ;
            else if (args[0] == "pull")
                ;
            else
                args[0] = "Invalid Operation";

            return args;
        }

        std::size_t send(const std::string & message)
        {
            return boost::asio::write(*m_session_sock_ptr, boost::asio::buffer(message));
        }

        std::size_t send(const std::vector <std::byte> & file)
        {
            return boost::asio::write(*m_session_sock_ptr, boost::asio::buffer(file));
        }

        void receive_files(const std::size_t num_files)
        {
            std::cout << "In receive files\n";
            for (std::size_t i{0}; i < num_files; i++)
            {
                std::cout << "\tReceiving file " << i << "\n";
                std::vector <std::byte> current_file;
                boost::asio::read(*m_session_sock_ptr, boost::asio::buffer(current_file));
                m_storage_ptr->push_back(current_file);
            }
        }

        void send_files()
        {
            send(std::to_string(m_storage_ptr->size()));

            for (auto current_file : *m_storage_ptr)
            {
                send(current_file);
            }

            m_storage_ptr->clear();
        }

    public:
        Stash_Session(boost::asio::ip::tcp::socket * session_sock_ptr, std::vector <std::vector <std::byte>> * storage_ptr)
            : m_session_sock_ptr {session_sock_ptr}, m_storage_ptr {storage_ptr}
        {
        }

        void start()
        {
            std::string data;
            std::cout << "!Before initial read\n";
            boost::asio::read_until(*m_session_sock_ptr, boost::asio::dynamic_buffer(data), ' ');
            std::cout << "!After initial read\n" << data;

            std::cout << "!Before initial read 2\n";

            std::vector <std::string> args;
            args.push_back(data);
            args.push_back("1");

            if (args[0] == "push")
            {
                receive_files(static_cast<std::size_t>(std::stoi(args[1])));
            }
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
        Stash_Server(int port = 3490)
            // Create acceptor for any ipv4 endpoint; opens, binds, and listens on endpoint
            : m_port{port}, m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        {
        }

        void run()
        {
            for(;;)
            {
                // Create and accept connection to socket
                boost::asio::ip::tcp::socket session_socket(m_io_context);
                m_acceptor.accept(session_socket);

                // Handle current session
                Stash_Session session (&session_socket, &m_storage);
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
    Stash_Server server;
    server.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
