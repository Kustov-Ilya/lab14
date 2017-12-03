#include <string>
#include <iostream>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/process.hpp>


namespace po = boost::program_options;
namespace bp = boost::process;
namespace ch = std::chrono;
struct Process{
	std::string Comand;
	size_t Wait;
	bool isWait;
	int exCd;

	Process() {
		exCd = 1;
		isWait = false;
		Wait = 0;
	}

	void StartProcessWithoutWait() {
		bp::ipstream out;
		bp::child Child(Comand, bp::std_out > out);
		{
			std::string line;
			while (out && std::getline(out, line) && !line.empty())
				std::cout << line << std::endl;
		}
		Child.wait();
		exCd = Child.exit_code();
	}


	void StartProcessWithWait(){
		bp::ipstream out;
		bp::child Child(Comand, bp::std_out > out);

		ch::system_clock::time_point begin{ ch::system_clock::now() };
		if (Child.wait_for(ch::seconds(Wait))) {
			ch::system_clock::time_point end{ ch::system_clock::now() };
			float dr{ ch::duration<float, std::ratio<1>>(end - begin).count() };
			Wait -= std::min(static_cast<size_t>(round(dr)), Wait);
		}
		else {
			Child.terminate();
			Wait = 0;
		}

		{
			std::string line;
			while (out && std::getline(out, line) && !line.empty())
				std::cout << line << std::endl;
		}

		exCd = (Wait==0)?1: Child.exit_code();
	}

	void StartProcess()
	{
		if (isWait) {
			StartProcessWithWait();
		}
		else {
			StartProcessWithoutWait();
		}
	}

};

int main(int argc, char const* const* argv)
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "help")
		("config", po::value<std::string>()->default_value("Debug"), "Сonfiguration")
		("install", "add install params")
		("package", "add packaging")
		("wait", po::value<size_t>(), "set timer")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	else {
		Process proc;
		proc.isWait = vm.count("wait");
		if (proc.isWait) {
			proc.Wait = vm["wait"].as<size_t>();
		}
		proc.Comand = "cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=_install -DCMAKE_BUILD_TYPE=" + vm["config"].as<std::string>();
		proc.StartProcess();
		if (proc.exCd == 0) {
			proc.Comand = "cmake --build _builds";
			proc.StartProcess();
		}
		if (proc.exCd == 0 && vm.count("package")) {
			proc.Comand = "cmake --build _builds --target package";
			proc.StartProcess();
		}
		if (proc.exCd == 0 && vm.count("install")) {
			proc.Comand = "cmake --build _builds --target install";
			proc.StartProcess();
		}
		return proc.exCd;
	}
}
