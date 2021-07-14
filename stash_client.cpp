#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/asio.hpp>

// g++ -I ~/HomeExt/boost_1_76_0 stash_client.cpp -o stash.out -std=c++17 -pthread

class Stash_Client
{
    private:
        std::string m_command;
        boost::asio::ip::tcp::resolver m_resolver;
        boost::asio::ip::tcp::socket m_socket;
        const std::filesystem::path m_stash_path;

        void on_resolve(boost::system::error_code error, boost::asio::ip::tcp::resolver::results_type endpoints)
        {
            std::cout << "Resolve: " << error.message() <<  "\n";
            boost::asio::async_connect(m_socket, endpoints, std::bind(&Stash_Client::on_connect, this, std::placeholders::_1, std::placeholders::_2));
        }


        void on_connect(boost::system::error_code error, boost::asio::ip::tcp::endpoint const& endpoint)
        {
            std::cout << "Connect: " << error.message() <<  "\n";
            std::cout << "Endpoint: " << endpoint << "\n";
            if (m_command == "push")
                boost::asio::async_write(m_socket, boost::asio::buffer("push 2\n"), std::bind(&Stash_Client::on_write, this, std::placeholders::_1, std::placeholders::_2));
            else if (m_command == "pull")
                boost::asio::async_write(m_socket, boost::asio::buffer("dummy pull\n"), std::bind(&Stash_Client::on_write, this, std::placeholders::_1, std::placeholders::_2));
        }

        void on_write(boost::system::error_code error, std::size_t bytes_transferred)
        {
            std::cout << "Write: " << error.message() << ", bytes transferred: " << bytes_transferred << "\n";
        }

        std::size_t get_stash_size ()
        {
            std::size_t i {0};
            for (auto& entry : std::filesystem::recursive_directory_iterator(m_stash_path))
            {
                i++;
            }
            return i;
        }

    public:
        Stash_Client(boost::asio::io_context& io_context, std::string command, std::filesystem::path stash_path = "Stash", std::string const& hostname = boost::asio::ip::host_name(), std::string port = "3490")
            : m_command{command}, m_stash_path{stash_path}, m_resolver(io_context), m_socket(io_context)
        {
            if (not std::filesystem::exists(m_stash_path))
            {
                std::filesystem::create_directory(m_stash_path);
            }
            m_resolver.async_resolve(hostname, port, std::bind(&Stash_Client::on_resolve, this, std::placeholders::_1, std::placeholders::_2));
        }

/*
        // Currently reads entire file into vector; send in pieces?
        void send_file(const std::filesystem::directory_entry & file)
        {
            std::cout << "\t Sending " << file.path().filename() << "\n";

            std::ifstream output_file (file.path(), std::ifstream::binary);
            std::vector <std::byte> output_buffer(m_buff_size);

            output_file.read(reinterpret_cast<char*>(output_buffer.data()), output_buffer.capacity());
            auto bytes_transferred = boost::asio::write(m_socket, boost::asio::buffer(output_buffer));

            std::cout << bytes_transferred;

            output_file.close();
        }

        void push()
        {
            std::cout << "Stash_Client\n\tStarting push to "<< m_server << "\n";
            connect();

            char header [128] {};
            strcpy(header, ("push "+std::to_string(get_stash_size())+" ").c_str());
            boost::asio::write(m_socket, boost::asio::buffer(header,128));

            for (auto& entry: std::filesystem::recursive_directory_iterator(m_stash_path))
            {
                send_file(entry);
            }

            std::cout << "\tFinished push.\n";
        }

        void pull()
        {
            std::cout << "Stash_Client: Starting pull from "<< m_server << "...\n";
            connect();

            boost::asio::write(m_socket, boost::asio::buffer("pull"));

            std::vector <char> data(m_buff_size);
            boost::asio::read(m_socket, boost::asio::buffer(data));

            std::cout << "Received: ";
            for (int i {0}; i < data.size(); i++)
                std::cout << data[i];
            std::cout << "\n";

            std::cout << "Stash_Client: Finished pull.\n";
        }
*/
};


// Check input to main
std::string verify_input(int argc, char** argv)
{
    if (argc != 2)
        throw "Usage is 'stash push' or 'stash pull'\n";
    std::string command {std::string(argv[1])};
    if (command != "push" and command !="pull")
    {
        throw "Usage is 'stash push' or 'stash pull'\n";
    }
    return command;
}


int main(int argc, char** argv)
{
  try
  {
    boost::asio::io_context io_context;
    Stash_Client client(io_context, verify_input(argc, argv));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
