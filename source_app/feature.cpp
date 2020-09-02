#include "Common.h"

std::string demo_perms(boost::filesystem::perms p)
{
	std::string string1;
	string1.append((p & boost::filesystem::perms::owner_read) != boost::filesystem::perms::no_perms ? "r" : "-")
		.append((p & boost::filesystem::perms::owner_write) != boost::filesystem::perms::no_perms ? "w" : "-")
		.append((p & boost::filesystem::perms::owner_exe) != boost::filesystem::perms::no_perms ? "x" : "-")
		.append((p & boost::filesystem::perms::group_read) != boost::filesystem::perms::no_perms ? "r" : "-")
		.append((p & boost::filesystem::perms::group_write) != boost::filesystem::perms::no_perms ? "w" : "-")
		.append((p & boost::filesystem::perms::group_exe) != boost::filesystem::perms::no_perms ? "x" : "-")
		.append((p & boost::filesystem::perms::others_read) != boost::filesystem::perms::no_perms ? "r" : "-")
		.append((p & boost::filesystem::perms::others_write) != boost::filesystem::perms::no_perms ? "w" : "-")
		.append((p & boost::filesystem::perms::others_exe) != boost::filesystem::perms::no_perms ? "x" : "-");
	return string1;
}

std::mutex m;
std::ofstream out;

std::ostream& operator<<(std::ostream& os, const char* text) {
	boost::unique_lock<std::mutex> lk(m);

	boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
	boost::posix_time::time_duration durObj = t.time_of_day();
	//Output to screen
	std::operator<<(os, " [");
	std::cout << durObj.minutes();
	std::operator<<(os, ":");
	std::cout << durObj.seconds();
	std::operator<<(os, ":");
	std::cout << durObj.total_milliseconds();
	std::operator<<(os, "] ");
	std::operator<<(os, text);

	//Output to log file
	std::operator<<(out, " [");
	out << durObj.minutes();
	std::operator<<(out, ":");
	out << durObj.seconds();
	std::operator<<(out, ":");
	out << durObj.total_milliseconds();
	std::operator<<(out, "] ");
	std::operator<<(out, text);
	out << std::endl;
	return os;
	//NOTE : I cannot use std::cout << "example" because this function will be overlapping
}