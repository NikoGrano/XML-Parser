#pragma once

#include <vector>
#include <tuple>
#include <boost/property_tree/ptree.hpp>

#include "../Engine.hpp"
#include "../Serializer.h"
#include "../common/Stream.h"

namespace Processor {
	class XmlWriter {
	public:
		XmlWriter(const std::string& fname);
		void saveTo(const std::string& destname);
		void toStdout();

	protected:
		std::vector<std::string> mStrings;
		boost::property_tree::ptree mTree;

		typedef std::vector<Serializer::dataBlock> dataArray;

		void collectStrings();
		uint16_t resolveString(const std::string& str) const;
		void treeWalker(const boost::property_tree::ptree& node);

		Engine::DataDescriptor BuildDescriptor(Serializer::rawDataBlock block, uint32_t prevOffset) const;
		Serializer::rawDataBlock serializeNode(const boost::property_tree::ptree& nodeVal, bool simple) const;
		std::string serializeSection(const boost::property_tree::ptree& section) const;
	};
}