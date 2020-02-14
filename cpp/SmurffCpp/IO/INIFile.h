// this code is based on https://github.com/Blandinium/inih/blob/master/cpp/INIReader.h 61bf1b3  on Dec 18, 2014

// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info: http://code.google.com/p/inih/

//This code is heavily changed to support our needs

#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

class INIFile
{
private:
   pt::ptree m_tree;

   static std::string add_index(std::string name, int index);

public:
   void read(const std::string& filename);
   void write(const std::string& filename);

public:
   template<typename T>
   T get(const std::string& section, const std::string& name, const T& default_value) const
   {
      return m_tree.get<T>(section + "." + name, default_value);
   }
   template<typename T>
   T get(const std::string& section, int idx,  const std::string& name, const T& default_value) const
   {
      return get(add_index(section, idx), name, default_value);
   }
   
public:
    // Returns true is section with name exists
    bool hasSection(const std::string &name) const;

public:
   template<typename T>
   void put(const std::string& section, const std::string& tag, const T& value)
   {
      m_tree.put(section + "." + tag, value);
   }
   template<typename T>
   void put(const std::string& section, int index, const std::string& tag, const T& value)
   {
      put(add_index(section, index), tag, value);
   }
};
