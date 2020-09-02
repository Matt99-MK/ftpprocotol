#pragma once

#ifndef _SERVER_H
#define _SERVER_H

#include "Common.h"
#include "Connection.h"

class Server
{
public:
	static unsigned int connections;
	static bool working;

	Server(boost::asio::io_service& io_service);
	Server(boost::asio::io_service& io_service, std::string IPv4, std::string Port);
	void handle_connection(boost::shared_ptr<Connection> pointer_to_Connection, const boost::system::error_code& error);
	void run();

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
	boost::shared_ptr<Connection> new_connection;
};
const std::vector<std::string> commands{ "ABOR", "ACCT", "ADAT","ALLO", "APPE", "RUTH", "AUTH+", "CCC", "CDUP", "CONE", "CWD" ,"DELE", "ENC" ,"EPRT", "EPSV", "FEAT" ,"HELP", "LANG", "LIST", "LPRT", "LPSV", "MDTM", "MIC" ,
	"MKD", "MLSD," "MLST", "MODE", "NLST", "NOOP", "OPTS" ,"PASS" ,"PASV", "PBSZ", "PBSZ+", "PORT", "PROT", "PROT+" ,"PWD", "QUIT", "REIN", "REST", "REST+", "RETR", "RMD", "RNFR", "RNTO", "SITE", "SIZE", "SMNT" ,"STAT" , "STOR", "STOU", "STRU" ,"SYST" ,"TYPE" ,"USER", "XCUP", "XCWD", "XMKD", "XPWD", "XRMD", "-N/A-" };

#endif
