#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/asio.hpp>

// g++ -I ~/HomeExt/boost_1_76_0 stash_client.cpp -o client.out -std=c++17 -pthread

class Stash_Client
{
    private:
        std::string m_server {boost::asio::ip::host_name()};
        std::string m_port {"3490"};
        const std::filesystem::path m_stash_path {"Stash"};

        boost::asio::ip::tcp::socket connect()
        {
            boost::asio::io_context io_context;
            boost::asio::ip::tcp::resolver resolver(io_context); //Associate <resolver> with <io_context>
            boost::asio::ip::tcp::resolver::results_type endpoints {resolver.resolve(m_server, m_port)}; //Get endpoints
            boost::asio::ip::tcp::socket socket(io_context); //Create socket and associate with <io_context>
            boost::asio::connect(socket, endpoints); // Connect <socket> and an acceptable <endpoint>; automatically loops through <endpoints>
            return socket;
        }

    public:
        Stash_Client()
        {
            if (not std::filesystem::exists(m_stash_path))
            {
                std::filesystem::create_directory(m_stash_path);
            }
            std::cout << "Constructor finished.\n";
        }

        void pull()
        {
            std::cout << "Stash_Client: starting pull from "<< m_server << "...";
            boost::asio::ip::tcp::socket socket {connect()};

            for (;;)
            {
                std::vector <char> data(128);
                boost::system::error_code error;

                size_t len = socket.read_some(boost::asio::buffer(data), error);
                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by peer.
                else if (error)
                    throw boost::system::system_error(error); // Some other error.
                else
                {
                    std::cout << "Received: ";
                    for (int i {0}; i < data.size(); i++)
                        std::cout << data[i];
                    std::cout << "\n";
                }
            }

            std::cout << "finished.\n";
        }
};

// Check input to main
void verify_input(int argc, char** argv)
{
    if ((argc != 2) or (std::string(argv[1]) != "push" and std::string(argv[1]) !="pull"))
    {
        throw "Invalid input; usage is\n\tstash push\n\tstash pull\n";
    }
}

int main(int argc, char* argv [])
{
    try
    {
        verify_input(argc, argv);
        Stash_Client client;
        client.pull();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (const char* e)
    {
        std::cerr << "\n" << e << std::endl;
    }
    catch (...)
    {
        std::cerr << "\nStash_client: an unexpected exception occurred.\n";
    }
    return 0;
}
