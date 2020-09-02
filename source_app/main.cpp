#include "Server.h"
extern std::ofstream out;

int main(int argc, char** argv)
{
	//Creating logging file
	boost::filesystem::path full_path(boost::filesystem::current_path());
	boost::filesystem::path full_path1 = full_path.parent_path().string() + full_path.root_directory().string() + "tests_logs" + full_path.root_directory().string();
	full_path1.append("logs.txt");
	out.open(full_path1.string());
	std::cerr << "Creating file " << full_path1.string() << std::endl;

	//Main Thread of working process
	std::cerr << "Main Thread: " << boost::this_thread::get_id() << std::endl;
	if (argc == 3)
	{
		try {
			boost::asio::io_service io_service;
			Server s(io_service, argv[1], argv[2]);
			s.run();
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
	}
	else
	{
		std::cerr << "Usage: ftp_server <adress> <port>" << std::endl;
		std::cerr << "Going default setting!" << std::endl;

		try {
			boost::asio::io_service io_service;
			Server s(io_service);
			s.run();
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
	}

	return 0;
}