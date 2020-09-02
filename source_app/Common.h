#pragma once

#ifndef _COMMON_H
#define _COMMON_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include<boost/enable_shared_from_this.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
#include<chrono>

#define DEFAULT_PORT 1001

std::ostream& operator<<(std::ostream& os, const char* text);
std::string demo_perms(boost::filesystem::perms p);

#endif