#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
#include "properties.h"
namespace pt = boost::property_tree;


void pup_static::load(const std::string &filename)
{
    // Create empty property tree object
    pt::ptree tree;

    // Parse the XML into the property tree.
    try {
      pt::read_xml(filename, tree);
    } catch(...){};

    nlimit = tree.get("puppy.time-limit", 60);
    klimit = tree.get("puppy.k-limit", 200);
    maxbaud = tree.get("puppy.max-baud", 1200);
    mdmstr = tree.get("puppy.modem-string", "ATE0V0M0X1S0=0");
    messages = tree.get("puppy.message-total", 10);
    msgsize = tree.get("puppy.message-size", 2560);
    callsize = tree.get("puppy.callers", 25);
    filepref = tree.get("puppy.file-prefix", "files/");

    pt::ptree subtree = tree.get_child("puppy.topics");
    for (auto z = subtree.begin(); z != subtree.end(); ++z)
      {
	pt::ptree leaf = (*z).second;
	topics.push_back(topic{leaf.get<std::string>("name"),
	      leaf.get<std::string>("desc")});
      }
    if (topics.empty())
      topics.push_back(topic{"generic","everything"});
}

#if 0
void pup_static::save(const std::string &filename)
{
    // Create an empty property tree object.
    pt::ptree tree;

    tree.put("puppy.time-limit", nlimit);
    tree.put("puppy.k-limit", klimit);
    tree.put("puppy.max-baud", maxbaud);
    tree.put("puppy.modem-string",mdmstr);
    tree.put("puppy.message-total", messages);
    tree.put("puppy.message-size", msgsize);
    tree.put("puppy.callers", callsize);
    tree.put("puppy.file-prefix", filepref);
    filepref = tree.get("puppy.file-prefix", "files/");
#if 0
    // Add all the modules. Unlike put, which overwrites existing nodes, add
    // adds a new node at the lowest level, so the "modules" node will have
    // multiple "module" children.
    BOOST_FOREACH(const std::string &name, m_modules)
        tree.add("debug.modules.module", name);
#endif
    
    // Write property tree to XML file
    pt::write_xml(filename, tree, std::locale(),
		  pt::xml_writer_make_settings<std::string>(' ',2));
}
#endif

int main()
{
  pup_static _ps;
  _ps.load("ps.dat");
  return 1;
}
