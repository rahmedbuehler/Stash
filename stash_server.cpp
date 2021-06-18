#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <vector>
#include <optional>

// g++ -I ~/HomeExt/boost_1_76_0 stash_server.cpp -o server.out -std=c++17 -pthread


// Add smart pointers

class Stash_Session : public std::enable_shared_from_this<Stash_Session>
{
    private:
        boost::asio::ip::tcp::socket m_session_sock;
        boost::asio::streambuf streambuf;

        /*
        std::vector <std::string> parse_header (boost::asio::streambuf streambuf, std::size_t header_size=128)
        {
            std::string current_arg {""};
            std::vector <std::string> args;
            for (int i{0}; i < header_size; i++)
            {
                if (header[i] == ' ' or i == header_size-1)
                {
                    args.push_back(current_arg);
                    current_arg = "";
                }
                else
                {
                    current_arg += header[i];
                }
            }

            std::cout << "Identified " << args.size() << " argument(s) in header\n";
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
        */

        std::size_t send(const std::string & message)
        {
            return boost::asio::write(m_session_sock, boost::asio::buffer(message));
        }

        std::size_t send(const std::vector <std::byte> & file)
        {
            return boost::asio::write(m_session_sock, boost::asio::buffer(file));
        }

        /*
        void receive_files(const std::size_t num_files)
        {
            std::cout << "In receive files\n";
            for (std::size_t i{0}; i < num_files; i++)
            {
                std::cout << "\tReceiving file " << i << "\n";
                std::vector <std::byte> current_file;
                boost::asio::read(m_session_sock, boost::asio::buffer(current_file));
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
        */

    public:
        Stash_Session(boost::asio::ip::tcp::socket&& session_sock)
            : m_session_sock(std::move(session_sock))
        {
        }

        void start()
        {
            boost::asio::async_read(m_session_sock, streambuf,
                [self = shared_from_this()] (boost::system::error_code error, std::size_t bytes_transferred)
            {
                std::cout << "In async handler; bytes transferred: " << bytes_transferred << "\n";
                std::cout << std::istream(&self->streambuf).rdbuf();
                std::cout << "\n";
                std::cout << "Leaving async handler\n";

            });

            /*
            std::size_t header_size {128};
            std::vector <std::string> args{parse_header(m_header, header_size)};

            if (args[0] == "push")
            {
                ;//receive_files(static_cast<std::size_t>(std::stoi(args[1])));
            }
            else if (args[0] == "pull")
                ;//send_files();
            */
        }
};


class Stash_Server
{
    private:
        int m_port;
        boost::asio::io_context* m_io_context_ptr;
        boost::asio::ip::tcp::acceptor m_acceptor;
        std::optional<boost::asio::ip::tcp::socket> socket;

    public:
        Stash_Server(boost::asio::io_context* io_context_ptr, std::uint16_t port = 3490)
            // Create acceptor for any ipv4 endpoint; opens, binds, and listens on endpoint
            : m_port{port}, m_io_context_ptr{io_context_ptr}, m_acceptor(*m_io_context_ptr, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        {
        }

        //void run()
        //{
            //for(;;)
            //{
                // Create and accept connection to socket
                //boost::asio::ip::tcp::socket session_socket(m_io_context);
                //m_acceptor.accept(session_socket);

                // Handle current session
                //Stash_Session session (*session_socket, &m_storage);
                //session.start();

                // Close socket
                //session_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                //session_socket.close();
            //}
        //}

        void async_accept()
        {
            socket.emplace(*m_io_context_ptr);
            m_acceptor.async_accept(*socket, [&] (boost::system::error_code error)
            {
                std::make_shared<Stash_Session>(std::move(*socket))->start();
                async_accept();
            });
        }

};

int main()
{
  try
  {
    boost::asio::io_context io_context;
    Stash_Server server(&io_context);
    server.async_accept();
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
