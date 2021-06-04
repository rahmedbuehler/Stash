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
        boost::asio::io_context m_io_context;
        boost::asio::ip::tcp::socket m_socket;
        std::string m_server {boost::asio::ip::host_name()};
        std::string m_port {"3490"};
        const std::filesystem::path m_stash_path {"Stash"};
        std::size_t m_buff_size {1024};

        void connect()
        {
            boost::asio::ip::tcp::resolver resolver(m_io_context); // Associate <resolver> with <io_context>
            boost::asio::ip::tcp::resolver::results_type endpoints {resolver.resolve(m_server, m_port)}; // Get endpoints
            boost::asio::connect(m_socket, endpoints); // Connect <socket> and an acceptable <endpoint>; automatically loops through <endpoints>
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
        Stash_Client() : m_socket(m_io_context) // Create socket and associate with <m_io_context>
        {
            if (not std::filesystem::exists(m_stash_path))
            {
                std::filesystem::create_directory(m_stash_path);
            }
        }

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
};

// Check input to main
void verify_input(int argc, char** argv)
{
    if ((argc != 2) or (std::string(argv[1]) != "push" and std::string(argv[1]) !="pull"))
    {
        throw "Usage is 'stash push' or 'stash pull'\n";
    }
}

int main(int argc, char* argv [])
{
    try
    {
        verify_input(argc, argv);
        Stash_Client client;
        client.push();
    }
    catch (std::exception& e)
    {
        std::cerr << "\tError in " << e.what() << std::endl;
    }
    catch (const char* e)
    {
        std::cerr << "\tError in " << e << std::endl;
    }
    catch (...)
    {
        std::cerr << "\nStash_client: an unexpected exception occurred.\n";
    }
    return 0;
}
