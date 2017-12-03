#include <string>
#include <iostream>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/process.hpp>

namespace po = boost::program_options;
namespace bp = boost::process;

class Process{
	std::string Comand;
	size_t timeout;
	bool isWait;
	int exCd;

public:
	Process() {
		exCd = 1;
		isWait = false;
		timeout = 0;
	}

	void AddComand(std::string str) {
		Comand = str;
	}

	void AddIsWait(bool is) {
		isWait = is;
	}

	bool CheckIsWait() {
		return isWait;
	}

	void setTimeout(size_t time) {
		timeout = time;
	}

	int getExCd() {
		return exCd;
	}

	void AllProcess()
	{
		bp::ipstream out;
		bp::child Child(Comand, bp::std_out > out);
		if (isWait) {
			if (!c.wait_for(std::chrono::seconds(timeout)));
			Child.terminate();
		}
		else {
			Child.wait();
		}
		exCd = Child.exit_code();
	}

};

int main(int argc, char const* const* argv)
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "help")
		("config", po::value<std::string>()->default_value("Debug"), "Сonfiguration")
		("install", "add install params")
		("pack", "add packaging")
		("timeout", po::value<size_t>(), "set timer")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	Process proc;
	proc.AddIsWait(vm.count("timeout"));
	if (proc.CheckIsWait())
		proc.setTimeout(vm["timeout"].as<size_t>());
	proc.AddComand("cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=_install -DCMAKE_BUILD_TYPE=" + vm["config"].as<std::string>());
	proc.AllProcess();
	if (proc.getExCd() == 0) {
		proc.AddComand("cmake --build _builds");
		proc.AllProcess();
	}
	if (proc.getExCd() == 0 && vm.count("pack")) {
		proc.AddComand("cmake --build _builds --target package");
		proc.AllProcess();
	}
	if (proc.getExCd() == 0 && vm.count("install")) {
		proc.AddComand("cmake --build _builds --target install");
		proc.AllProcess();
	}
	return proc.getExCd();
}
