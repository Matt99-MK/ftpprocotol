#include "Server.h"

unsigned int Server::connections = 0;
bool Server::working = false;

Server::Server(boost::asio::io_service& io_service)
	: io_service_(io_service), acceptor_((io_service), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), DEFAULT_PORT))
{
	new_connection = Connection::Create_Connection(io_service_);
	boost::asio::socket_base::reuse_address option(true);
	acceptor_.set_option(option);
	acceptor_.async_accept(new_connection->Socket(), boost::bind(&Server::handle_connection, this, new_connection, boost::asio::placeholders::error));
	//Thanks to async listening that line of code is still running. In not asyn code is stopped one line above.
	Server::connections = 0;
	Server::working = true;
}

Server::Server(boost::asio::io_service& io_service, std::string IPv4, std::string Port)
	: io_service_(io_service), acceptor_(io_service)
{
	boost::asio::ip::address address = boost::asio::ip::make_address_v4(IPv4);
	boost::asio::ip::tcp::endpoint ep(address, std::stoi(Port));
	acceptor_.open(ep.protocol());
	acceptor_.bind(ep);
	acceptor_.listen();
	new_connection = Connection::Create_Connection(io_service_);
	boost::asio::socket_base::reuse_address option(true);
	acceptor_.set_option(option);
	acceptor_.async_accept(new_connection->Socket(), boost::bind(&Server::handle_connection, this, new_connection, boost::asio::placeholders::error));
	//Thanks to async listening that line of code is still running. In non asyn code is stopped one line above.
	Server::connections = 0;
	Server::working = true;
}

void Server::handle_connection(boost::shared_ptr<Connection> pointer_to_Connection, const boost::system::error_code& error)
{
	try {
		boost::asio::ip::address address;
		pointer_to_Connection->Socket().remote_endpoint().address(address);
		std::cerr << std::endl;
		std::cerr << "Handling Connection number " << Server::connections << std::endl;
		std::cerr << "From Remote Address: " << address.to_string() << std::endl;
		std::cerr << "Port : " << pointer_to_Connection->Socket().remote_endpoint().port() << std::endl << std::endl;

		if (!error)
		{
			++Server::connections;
			pointer_to_Connection->Ignite();
			new_connection = Connection::Create_Connection(io_service_);
			acceptor_.async_accept(new_connection->Socket(), boost::bind(&Server::handle_connection, this, new_connection, boost::asio::placeholders::error));
		}
		else
		{
			std::cerr << "FAILED!" << std::endl;
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void Server::run()
{
	auto count = std::thread::hardware_concurrency() * 2;
	// Create a pool of threads to run all of the io_contexts.
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < count; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&boost::asio::io_context::run, &io_service_)));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}