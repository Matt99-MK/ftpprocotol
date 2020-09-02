#include "Connection.h"
#include <boost/make_shared.hpp>
#include "boost/filesystem.hpp"

void Connection::handle_LIST(const boost::system::error_code& error)
{
	boost::asio::ip::address address;
	this->socketp_->remote_endpoint().address(address);
	std::cerr << std::endl;
	std::cerr << "Connection to PASV_SOCKET " << std::endl;
	std::cerr << "From Remote Address: " << address.to_string() << std::endl;
	std::cerr << "Port : " << this->socketp_->remote_endpoint().port() << std::endl;

	//START SENDING DATA
	boost::asio::async_write(socket_, boost::asio::buffer("150 Opening BINARY mode data connection.\r\n"), m_strand.wrap(boost::bind(&Connection::handle_write, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
	//Parse
	boost::filesystem::directory_iterator end_itr;
	for (boost::filesystem::directory_iterator itr(relative); itr != end_itr; ++itr)
	{
		//Prepare send structure in working directory
		std::string sender;
		auto ftime = boost::filesystem::last_write_time(itr->path());
		sender.append("modify=").append(boost::posix_time::to_iso_string(boost::posix_time::from_time_t(ftime)));
		sender.append(";");
		boost::filesystem::file_status s = boost::filesystem::status(itr->path());
		sender.append("perm=").append(demo_perms(boost::filesystem::status(itr->path().string()).permissions()));
		sender.append(";");

		if (!boost::filesystem::is_directory(itr->path()))
		{
			sender.append("size=").append(std::to_string(boost::filesystem::file_size(itr->path()))).append(";");
			sender.append("type=").append(std::to_string(s.type()));
		}
		else
		{
			sender.append("type=dir");
		}

		sender.append(";");
		sender.append("UNIX.owner=UNKNOWN;UNIX.group=UNKNOW; ");
		sender.append(itr->path().filename().string());
		sender.append("\r\n");
		//Send final structure
		boost::asio::async_write(*socketp_, (boost::asio::buffer(sender.c_str(), sender.size())), m_strand.wrap(boost::bind(&Connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
	}

	socketp_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	socketp_->cancel();
	socketp_->close();
	socketp_ = boost::make_shared<boost::asio::ip::tcp::socket>(io_context_);

	boost::asio::async_write(socket_, boost::asio::buffer("226 Transfer Complete\r\n"), m_strand.wrap(boost::bind(&Connection::handle_write, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));

	relative = current_path;
}

void Connection::handle_RETR(const boost::system::error_code& error, std::string path)
{
	boost::asio::ip::address address;
	this->socketp_->remote_endpoint().address(address);
	std::cerr << std::endl;
	std::cerr << "Connection to PASV_SOCKET " << std::endl;
	std::cerr << "From Remote Address: " << address.to_string() << std::endl;
	std::cerr << "Port : " << this->socketp_->remote_endpoint().port() << std::endl;

	//START
	boost::asio::async_write(socket_, boost::asio::buffer("150 Opening BINARY mode data connection.\r\n"), m_strand.wrap(boost::bind(&Connection::handle_write, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
	//-------
	//Sending file
	boost::array<char, 4096> buf;
	std::ifstream fs;
	fs.open(path, std::ios::binary);
	while (true)
	{
		if (fs.eof() == false)
		{
			fs.read(buf.c_array(), (std::streamsize)buf.size());
			if (fs.gcount() <= 0)
			{
				std::cout << "read file error " << std::endl;
				break;
			}

			boost::asio::write(*socketp_, boost::asio::buffer(buf.c_array(), fs.gcount()), boost::asio::transfer_all());

			continue;
		}
		break;
	}
	//END
	socketp_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	socketp_->cancel();
	socketp_->close();
	socketp_ = boost::make_shared<boost::asio::ip::tcp::socket>(io_context_);

	//END 2
	boost::asio::async_write(socket_, boost::asio::buffer("226 Transfer Complete\r\n"), m_strand.wrap(boost::bind(&Connection::handle_write, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
}