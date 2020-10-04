#include "Writer.h"
#include "../common/Base64.h"
#include "../Serializer.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <boost/assign.hpp>

namespace Processor {
	using boost::property_tree::ptree;
	using namespace Engine;
    using namespace Serializer;

	XmlWriter::XmlWriter(const std::string& fname)
	{
		try {
			boost::property_tree::xml_parser::read_xml(fname, mTree, boost::property_tree::xml_parser::trim_whitespace);
		} catch(...) {
			throw std::runtime_error("XML parsing error");
		};
		
        if (mTree.size() != 1) {
			throw std::runtime_error("XML file must contain only 1 root level node");
        }

		mTree.swap(mTree.begin()->second); // Swapping the whole tree to its first node
	}

	void XmlWriter::treeWalker(const ptree& node) {
		for (auto subNode=node.begin(); subNode != node.end(); ++subNode) {
			mStrings.push_back(subNode->first);
			treeWalker(subNode->second);
		}
	}

	void XmlWriter::collectStrings() {
		mStrings.clear();
		treeWalker(mTree);
		std::sort(mStrings.begin(), mStrings.end());
		mStrings.erase(std::unique(mStrings.begin(), mStrings.end()),mStrings.end());

		static std::vector<std::string> namesToClear = boost::assign::list_of\
			("<xmlcomment>")("row0")("row1")("row2")("row3");	// utility nodes
		for (auto str=namesToClear.begin(); str!=namesToClear.end(); ++str) {
			auto strIdx = std::find(mStrings.begin(), mStrings.end(), *str);
			if (strIdx != mStrings.end()) {
				mStrings.erase(strIdx);
            }
		}
	}

	uint16_t XmlWriter::resolveString(const std::string& str) const {
		auto pos = std::find(mStrings.begin(), mStrings.end(), str);
		if (pos == mStrings.end()) {
			throw std::runtime_error("String key not found!");
        }
		size_t strIdx = std::distance(mStrings.begin(), pos);
		if (strIdx > std::numeric_limits<uint16_t>::max()) {
			throw std::runtime_error("String table overflow!");
        }

		return static_cast<uint16_t>(strIdx);
	}

	// 'simple' indicates that we need the node's exact value, even if it has children
	rawDataBlock XmlWriter::serializeNode(const boost::property_tree::ptree& node_value, bool simple) const
	{
		if (node_value.size() == MATRIX_NROWS) { // Matrix
			std::vector<boost::optional< const ptree& > > rows;
			for (int i=0; i < MATRIX_NROWS; ++i) {
				auto row = node_value.get_child_optional("row"+boost::lexical_cast<std::string>(i));
				if (!row) {
					break; // Nope
                }
    
				rows.push_back(row);
			}

			if (rows.size() == MATRIX_NROWS) { // Found all 4 rows
				std::stringstream buffer;
				for (auto it=rows.begin(); it!=rows.end(); ++it) {
					rawDataBlock block = PackBuffer((*it)->data());
					assert(block.type == TYPE_Float);
					buffer << block.data;
				}

				return rawDataBlock(TYPE_Float, buffer.str());
			}
		}

		if (!simple && node_value.size() && (!node_value.get_child_optional("<xmlcomment>"))) { // has sub-nodes
			return rawDataBlock(TYPE_Section, serializeSection(node_value));
		}

		if (!node_value.get("<xmlcomment>", "").compare("BW_String")) {
			return rawDataBlock(TYPE_String, node_value.data()); 
        }

		return PackBuffer(node_value.data());
	}

	Engine::DataDescriptor XmlWriter::BuildDescriptor(rawDataBlock block, uint32_t prevOffset) const
	{
		if (block.data.length() > std::numeric_limits<uint32_t>::max())
			throw std::runtime_error("Data block is too large");

		return DataDescriptor(block.type, prevOffset + static_cast<uint32_t>(block.data.length()));
	}

	void XmlWriter::saveTo(const std::string& destname) {
		collectStrings();

		std::stringstream outbuf;
		Stream::IO::StreamBufWriter outstream(outbuf.rdbuf());
		outstream.put(Engine::PACKED_SECTION_MAGIC);
		outstream.put<uint8_t>(0);
		for (auto it = mStrings.begin(); it!= mStrings.end(); ++it)
			outstream.putString(*it);
		outstream.put<uint8_t>(0);

		outstream.putString(serializeSection(mTree), false);

		std::ofstream mFile;
		mFile.open(destname, std::ios::binary);
		if (!mFile.is_open())
			throw std::runtime_error("Can't open the file");
		mFile << outbuf.rdbuf();
		mFile.close();
	}

	std::string XmlWriter::serializeSection(const ptree& node) const {
		std::stringstream _ret;
		Stream::IO::StreamBufWriter ret(_ret.rdbuf());

		rawDataBlock ownData = serializeNode(node, true);
		dataArray childData;
		for (auto it=node.begin(); it!=node.end(); ++it) {
			if (!it->first.compare("<xmlcomment>")) { // Will skip comments.
				continue;
            }
			childData.push_back(dataBlock(resolveString(it->first), serializeNode(it->second, false)));
		}

		if (childData.size() > std::numeric_limits<uint16_t>::max()) {
			throw std::runtime_error("Too many children nodes!");
        }

		DataDescriptor ownDescriptor = BuildDescriptor(ownData, 0);
		ret.put<uint16_t>(static_cast<uint16_t>(childData.size()));
		ret.put<DataDescriptor>(ownDescriptor);

		uint32_t currentOffset = ownDescriptor.offset();
		for (auto it = childData.begin(); it != childData.end(); ++it) {
			DataNode bwNode;
			bwNode.nameIdx = it->stringId;
			bwNode.data = BuildDescriptor(it->data, currentOffset);
			ret.put<DataNode>(bwNode);
			currentOffset = bwNode.data.offset();
		}

		ret.putString(ownData.data, false);
		for (auto it = childData.begin(); it != childData.end(); ++it) {
			ret.putString(it->data.data, false);
		}

		return _ret.str();
	}
}