#include "Connection.h"
#include "Server.h"
#include <boost/make_shared.hpp>
#include "boost/filesystem.hpp"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

Connection::Connection(boost::asio::io_context& io_context) : io_context_(io_context), socket_(io_context), is(&read_buffer), acceptor_((io_context),
	boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0)), m_strand(io_context)
{
}

boost::shared_ptr<Connection> Connection::Create_Connection(boost::asio::io_context& io_context)
{
	return boost::shared_ptr<Connection>(new Connection(io_context));
}

boost::asio::ip::tcp::socket& Connection::Socket(void)
{
	return socket_;
}

Connection::~Connection()
{
	std::cerr << "Leaving Connection from thread " << boost::this_thread::get_id() << std::endl;
}

void Connection::Ignite(void) // Setting configuration
{
	try {
		//Socket option
		boost::asio::socket_base::keep_alive option(true);
		socket_.set_option(option);

		//Init startup variables
		started_ = true;
		logged_ = false;
		prelogged_ = false;

		//Init working directory to listing
		current_path = boost::filesystem::current_path();
		current_path = current_path.parent_path().string() + current_path.root_directory().string() + "config.txt";
		std::ifstream out;
		out.open(current_path.string());
		std::string ss;
		out >> ss;
		std::cout << ss;
		current_path = ss;

		pwd = current_path.root_directory().string();
		path_root = current_path.root_directory();
		relative = current_path;

		//New Connection and Welcome announcement
		if (Server::working == true)
		{
			s.append("220 ");
			s.append(boost::asio::ip::address_v4::address_v4().to_string().append(" FTPServer ready for service\r\n"));
			boost::asio::async_write(socket_, boost::asio::buffer(s), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			s.append("421 ");
			s.append(boost::asio::ip::address_v4::address_v4().to_string().append(" FTPServer not available\r\n"));
			boost::asio::async_write(socket_, boost::asio::buffer(s), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}

		do_init();
	}

	catch (std::exception& exception)
	{
		std::cout << exception.what() << std::endl;
	}
}

void Connection::do_init(void)
{
	try {
		boost::asio::async_read_until(socket_, read_buffer, '\r\n', boost::bind(&Connection::handle_read, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		//NOTE: Thanks to shared_from_this() that object isn't deleted, because we pass pointer to another function
	}
	catch (const std::exception& exception)
	{
		std::cout << exception.what();
	}
}

void Connection::handle_read(const boost::system::error_code& err, std::size_t bytes_transferred) //MAIN THREAD
{
	//Make sure s string is clear
	s.clear();
	try {
		std::cerr << "handle_read Thread: " << boost::this_thread::get_id() << std::endl;
		//Process agreement to pass request
		bool is_acceptable = false;
		while (!(is.peek() == -1) && is_acceptable != true)
		{
			if (isgraph(is.peek()))
				is_acceptable = true;
			else
				is.get();
		}

		if (!is_acceptable) //If next charackter is carriage return or clinet presses ENTER.
		{
			std::cout << "input state : " << is.bad() << is.fail() << is.eof() << is.good() << std::endl;
			if (is.fail() || is.eof()) //If user kill connection or press nothing
			{
				return; //Return and invoke Destructor
			}

			//--- Wait for the next command
			boost::asio::async_read_until(socket_, read_buffer, '\r\n', boost::bind(&Connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			std::cout << "";
			std::cout << "input state : " << is.bad() << is.fail() << is.eof() << is.good() << std::endl;
			//Show whole input stream content
			const unsigned char* p1 = static_cast<const unsigned char*>(read_buffer.data().data());
			for (int i = 0; i < bytes_transferred - 1; ++i)
			{
				std::cout << p1[i];
			}
			std::cout << std::endl;
			is >> s;

			if (s.empty())
			{
				return;
			}
			else
			{
				boost::to_upper(s);
				//Creating reply to client

				reply(s);

				//Input state after creating reply
				std::cout << "input state : " << is.bad() << is.fail() << is.eof() << is.good() << std::endl;

				//Flush stream and clean input variables
				getline(is, s);
				is.clear();
				s.clear();

				//--- Wait for the next command
				boost::asio::async_read_until(socket_, read_buffer, '\r\n', boost::bind(&Connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
		}
	}

	catch (const std::exception& exception)
	{
		std::cout << exception.what();
	}
}

void Connection::handle_write(const boost::system::error_code& err, std::size_t bytes_transferred)
{
	std::cerr << "handle_write Thread: " << boost::this_thread::get_id() << std::endl << std::endl;
}

void Connection::reply(std::string forepart)
{
	try {
		bool checker = commands.end() == std::find(commands.begin(), commands.end(), forepart);
		if (checker) //Check availible commands
		{
			forepart.insert(0, std::string("500 "));
			forepart.append(" not understood\r\n");
			boost::asio::async_write(socket_, boost::asio::buffer(forepart.c_str(), forepart.size()), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		//Check log authentication
		if (forepart == "PASS" && logged_ == true)
		{
			boost::asio::async_write(socket_, boost::asio::buffer("503 You are already logged in!\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		if (forepart == "USER" && logged_ == true)
		{
			boost::asio::async_write(socket_, boost::asio::buffer("500 Bad sequence of commands\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		if ((forepart != "USER" && prelogged_ == false) || (forepart != "PASS" && (logged_ == false && prelogged_ == true))) //Check is logged
		{
			boost::asio::async_write(socket_, boost::asio::buffer("530 please login with USER and PASS\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}

		//Command request dispacher
		if (forepart == "CWD")
		{
			std::string sender;
			is >> sender;

			if (sender.front() != '\\')
				sender.insert(0, "\\");

			relative = current_path;

			determiner = relative;
			determiner.append(sender);

			if (!boost::filesystem::exists(determiner)) //If path exist
			{
				if (sender.front() != '\\')
					sender.insert(0, "\\");
				pwd += sender;
				relative.append(pwd);
			}
			else
			{
				pwd = sender;
				relative.append(pwd);
			}

			boost::asio::async_write(socket_, boost::asio::buffer("200 directory changed \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

			return;
		}
		else if (forepart == "FEAT") //I don't yet support feature expose
		{
			boost::asio::async_write(socket_, boost::asio::buffer("500 Command not supported yet\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		else if (forepart == "HELP")
		{
			//To implement in future
		}
		else if (forepart == "LIST")
		{
			acceptor_.async_accept(*socketp_, boost::bind(&Connection::handle_LIST, shared_from_this(), boost::asio::placeholders::error));
			return;
		}
		else if (forepart == "MKD")
		{
			//To implement in future
			return;
		}
		else if (forepart == "RETR")
		{
			std::string file;
			is >> file;
			acceptor_.async_accept(*socketp_, boost::bind(&Connection::handle_RETR, shared_from_this(), boost::asio::placeholders::error, current_path.string() + pwd + path_root.string() + file));
			return;
		}
		else if (forepart == "MODE")
		{
			//To implement in future
			return;
		}
		else if (forepart == "PASS")
		{
			is >> s;

			boost::asio::async_write(socket_, boost::asio::buffer("230 User logged in\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			logged_ = true;
			return;
		}
		else if (forepart == "PASV")
		{
			boost::asio::ip::tcp::endpoint pasv_endpoint = socket_.local_endpoint();
			socketp_ = boost::make_shared<boost::asio::ip::tcp::socket>(io_context_);
			boost::asio::ip::tcp::endpoint pasv_endpoint_port = acceptor_.local_endpoint();
			std::string probe = pasv_endpoint.address().to_v4().to_string();
			boost::char_separator<char> sep(".");
			boost::tokenizer< boost::char_separator<char> > token(probe, sep);
			std::string pasv_message("227 Entering Passive Mode ");
			for (auto tokens = token.begin(); tokens != token.end(); ++tokens)
			{
				pasv_message.append(*tokens);
				pasv_message.append(",");
			}
			pasv_message.append(std::to_string(pasv_endpoint_port.port() / 256));
			pasv_message.append(",");
			pasv_message.append(std::to_string(pasv_endpoint_port.port() % 256));
			pasv_message.append("\r\n");
			boost::asio::async_write(socket_, boost::asio::buffer(pasv_message.c_str(), pasv_message.size()), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		else if (forepart == "PORT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "PWD")
		{
			std::string forepart = pwd;
			forepart.insert(0, std::string("257 \""));
			forepart.append("\" is the current directory \r\n");
			std::cout << "forepart " << forepart << std::endl;

			boost::asio::async_write(socket_, boost::asio::buffer(forepart.c_str(), forepart.size()), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		else if (forepart == "QUIT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "RMD")
		{
			//To implement in future
			return;
		}
		else if (forepart == "SIZE")
		{
			//To implement in future
			return;
		}
		else if (forepart == "STAT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "SYST")
		{
			boost::asio::async_write(socket_, boost::asio::buffer("215 UNIX Type: L8\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			return;
		}
		else if (forepart == "TYPE") //Inform about data type transferred(ASCII for text files, binary image for files executables etc)
		{
			if (read_buffer.size() == 2)
			{
				boost::asio::async_write(socket_, boost::asio::buffer("500 'TYPE' not understood \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				return;
			}
			else
			{
				is >> s;
				if (s == "A")
				{
					boost::asio::async_write(socket_, boost::asio::buffer("200 'TYPE' set to 'A' \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
				else if (s == "E")
				{
					boost::asio::async_write(socket_, boost::asio::buffer("200 'TYPE' set to 'E' \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
				else if (s == "I")
				{
					boost::asio::async_write(socket_, boost::asio::buffer("200 'TYPE' set to 'I' \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
				else if (s == "L")
				{
					boost::asio::async_write(socket_, boost::asio::buffer("200 'TYPE' set to 'L' \r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
				else
				{
					boost::asio::async_write(socket_, boost::asio::buffer("504 'TYPE' not implemented for that parameter\r\n"), boost::bind(&Connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
				return;
			}
		}
		else if (forepart == "USER")
		{
			is >> s;
			if (s._Equal("anonymous"))
			{
				s.insert(0, "331 Password required for ");
				s.append("\r\n");

				boost::asio::async_write(socket_, boost::asio::buffer(s.c_str(), s.size()), boost::bind(&Connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
			prelogged_ = true;
			return;
		}
		else if (forepart == "ABOR")
		{
			//To implement in future
			return;
		}
		else if (forepart == "ACCT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "ADAT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "ALLO")
		{
			//To implement in future
			return;
		}
		else if (forepart == "APPE")
		{
			//To implement in future
			return;
		}
		else if (forepart == "RUTH")
		{
			//To implement in future
			return;
		}
		else if (forepart == "AUTH")
		{
			//To implement in future
			return;
		}
		else if (forepart == "MLST")
		{
			//To implement in future
			return;
		}
		else if (forepart == "NOOP")
		{
			//To implement in future
			return;
		}
		else if (forepart == "OPTS")
		{
			//To implement in future
			return;
		}
		else if (forepart == "PBSZ")
		{
			//To implement in future
			return;
		}
		else if (forepart == "DELE")
		{
			//To implement in future
			return;
		}
		else if (forepart == "EPSV")
		{
			//To implement in future
			return;
		}
		else if (forepart == "LPRT")
		{
			//To implement in future
			return;
		}
		else if (forepart == "REIN")
		{
			//To implement in future
			return;
		}
		else if (forepart == "REST")
		{
			//To implement in future
			return;
		}
		else if (forepart == "RMD")
		{
			//To implement in future
			return;
		}
		else if (forepart == "RNTO")
		{
			//To implement in future
			return;
		}
		else if (forepart == "STOR")
		{
			//To implement in future
			return;
		}
		else if (forepart == "STRU")
		{
			//To implement in future
			return;
		}
		else if (forepart == "XMKD")
		{
			//To implement in future
			return;
		}
	}
	catch (const std::exception& exception)
	{
		std::cout << exception.what();
	}
	return;
}