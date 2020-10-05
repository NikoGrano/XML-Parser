#pragma once

#include <vector>

#include "../common/Stream.h"
#include "../Engine.hpp"
#include <boost/property_tree/ptree.hpp>

namespace Processor {
    class XmlReader {
        public:
            XmlReader(const std::string& fname);
            void saveTo(const std::string& destName) const;
            void toStdout();

        protected:
            Stream::IO::StreamReader mStream;
            std::vector<std::string> mStrings;
            boost::property_tree::ptree mTree;

            void ReadStringTable();
            boost::property_tree::ptree ReadSection();
            void readData(Engine::DataDescriptor descr, boost::property_tree::ptree& currentNode, uint32_t previous);
    };
}
