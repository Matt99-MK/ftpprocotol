#pragma once

#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "Common.h"

class Connection : public boost::enable_shared_from_this<Connection>
{
public:
	Connection(boost::asio::io_context& io_context);
	~Connection();
	static boost::shared_ptr<Connection> Create_Connection(boost::asio::io_context& io_context);
	boost::asio::ip::tcp::socket& Socket(void);
	void handle_read(const boost::system::error_code& err, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& err, std::size_t bytes_transferred);
	void handle_LIST(const boost::system::error_code& error);
	void handle_RETR(const boost::system::error_code& error, std::string path);
	void reply(std::string reply);
	void do_init(void);
	void Ignite(void);

private:

	std::string pwd;
	boost::filesystem::path current_path;
	boost::filesystem::path path_root;
	boost::filesystem::path relative;
	boost::filesystem::path determiner;

	boost::asio::streambuf read_buffer;
	std::string s;
	std::string s_tem;
	std::istream is;

	bool logged_;
	bool started_;
	bool prelogged_;

	boost::asio::io_context& io_context_;
	boost::asio::ip::tcp::socket socket_;
	boost::shared_ptr<boost::asio::ip::tcp::socket> socketp_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::io_context::strand m_strand;
};

#endif