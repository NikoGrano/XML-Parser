#include "processor/Reader.h"
#include "processor/Writer.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <boost/thread.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
namespace bpo = boost::program_options;

#include <vector>
#include <algorithm>

void convert(std::string src, std::string dest, bool doPack, bool stdout) {
	if(stdout) {
		doPack ? Processor::XmlWriter(src).toStdout() : Processor::XmlReader(src).toStdout();
	} else {
		doPack ? Processor::XmlWriter(src).saveTo(dest) : Processor::XmlReader(src).saveTo(dest);
	}
}

std::string FindCommonPrefix(const std::vector<path>& paths) {
	std::string s1, s2;
	if (paths.size() < 2) {
		return "";
    }

	auto lenCmp = [](const path& p1, const path& p2){return p1.string().length() < p2.string().length(); };
	s1 = (*std::min_element(paths.begin(), paths.end(), lenCmp)).string();
	s2 = (*std::max_element(paths.begin(), paths.end(), lenCmp)).string();

	for (size_t i=0; i<s1.length(); ++i) {
		if (s1[i] != s2[i]) {
			s1 = s1.substr(0, i);
			break;
		}
	}

	path s1p(s1);
	if (!exists(s1p)) {
		s1 = s1p.parent_path().string();
    }

	return s1;
}

int main(int argc, char* argv[]) {
	bpo::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show this help")
		("encode", "Encode files")
		("threads", bpo::value<int>()->default_value(boost::thread::hardware_concurrency() + 1), "Thread count. Default = n_cpu_cores + 1")
		("input", bpo::value<std::vector<std::string>>(), "Input files/directories")
		("output", bpo::value<std::string>()->default_value("output/"), "Directory to output files. Default: output")
		("stdout", "Output to stdout. Single file only!")
		;

	bpo::positional_options_description po;
	po.add("input", -1);

	bpo::variables_map vm;
	try{
		bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(po).run(), vm);
		bpo::notify(vm);
	} catch (const std::exception&) {
		// ignoring arg parsing errors
	}

	if(!vm.count("stdout")) {
		std::cout << "XML Parser" << std::endl;
	}

	if (vm.count("help") || vm["input"].empty()) {
		std::cout << "Usage: " << argv[0] << " [options] directories|files\n";
		std::cout << desc;
		return 0;
	}

	bool doPack = (vm.count("encode") != 0);
	auto inputPaths = vm["input"].as< std::vector<std::string> >();
	std::string srcfile = ""; 

	std::string destdir = vm["output"].as<std::string>();
	destdir.append("/");

	std::vector<path> paths, valid_paths;

	if(!vm.count("stdout")) {
	std::cout << "Collecting files... " ;
	}
	
	try {
		for (auto it=inputPaths.begin(); it != inputPaths.end(); ++it) {
			path current = path(*it);
			if (!exists(current)) {
				std::cout << "Path '" << *it << "' unavailable, skipping" << std::endl;
			}

			if (is_directory(current)) {
				std::copy(recursive_directory_iterator(current, symlink_option::recurse), 
				recursive_directory_iterator(), back_inserter(paths));
			}

			if (is_regular_file(current)) {
				paths.push_back(current);
			}
		}

		if(!vm.count("stdout")) {
			std::cout << "filtering... ";
		}
		std::copy_if(paths.begin(), paths.end(), std::back_inserter(valid_paths),[](const path& p){return is_regular_file(p);});

		if(!vm.count("stdout")) {
			std::cout << "done. \nFound " << valid_paths.size() << " file(s), processing to " << destdir << std::endl;
		}

		if (valid_paths.empty()) {
			std::cout << "Nothing to do!" << std::endl;
			return 2;
		}

		std::string commonPrefix =	FindCommonPrefix(valid_paths);
		if (commonPrefix.empty()) {
			commonPrefix = path(valid_paths[0]).parent_path().string();
		}

		int nThreads = vm["threads"].as<int>();
		if(!vm.count("stdout")) {
			std::cout << "Starting a pool with " << nThreads << " workers" << std::endl;
		}

		auto workerThread = [&](int num)
		{
			for (size_t i=num; i < valid_paths.size(); i += nThreads) {
				std::string rel_path = valid_paths.at(i).string().substr(commonPrefix.length());
				std::string target_path = destdir + rel_path;

				path _target_path = path(target_path);
				_target_path = _target_path.parent_path();
				if (!exists(_target_path) && !vm.count("stdout")) {
					create_directories(_target_path);
				}

				try {
					convert(valid_paths.at(i).string(), target_path, doPack, vm.count("stdout"));
					if(!vm.count("stdout")) {
						std::cout << "+";
					}
				} catch (const std::exception& e) {
						std::cout << "!";
				}
			}
		};

		boost::thread_group pool;
		for (int i=0; i<nThreads; ++i) {
			pool.create_thread(boost::bind<void>(workerThread, i));
		}

		pool.join_all();
	} catch (const std::exception& e){
		std::cerr << "ERROR: " << e.what() << std::endl;
		return -1;
	}

	//std::cout << std::endl << "Done." << std::endl;
	return 0;
}