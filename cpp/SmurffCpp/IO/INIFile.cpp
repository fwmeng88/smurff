#include <fstream>

#include <Utils/Error.h>

#include "INIFile.h"

#include <boost/property_tree/ini_parser.hpp>

namespace smurff
{

pt::ptree INIFile::makeUnique(const pt::ptree &pt)
{
   if (pt.size() <= 1)
      return pt;

   std::map<pt::ptree::key_type, size_t> counts;
   for (const auto &el : pt)
      counts[el.first]++;
   // remove counts of 1 or less
   for (auto &c : counts)
      if (c.second <= 1)
         c.second = 0;

   pt::ptree ret(pt.data());
   for (const auto &el : pt)
   {
      pt::ptree::key_type key = el.first;
      if (counts[key] > 0)
      {
         counts[key]--;
         key += "_" + std::to_string(counts[key]);
      }
      ret.push_back({key, makeUnique(el.second)});
   }

   return ret;
}

void INIFile::read(const std::string &filename)
{
   std::ifstream file;
   file.open(filename, std::ios::in);
   pt::ini_parser::read_ini(file, m_tree);
}

void INIFile::write(const std::string &filename)
{
   std::ofstream file;
   file.open(filename, std::ios::trunc);
   pt::ini_parser::write_ini(file, makeUnique(m_tree));
}

bool INIFile::hasSection(const std::string &name) const
{
   return m_tree.get_child_optional(name) != boost::none;
}

bool INIFile::hasDataSet(const std::string &name, const std::string &tag) const
{
   THROWERROR_NOTIMPL();
}

std::shared_ptr<Matrix> INIFile::getMatrix(const std::string& section, const std::string& tag) const
{
   THROWERROR_NOTIMPL();
}

std::shared_ptr<Vector> INIFile::getVector(const std::string& section, const std::string& tag) const
{
   THROWERROR_NOTIMPL();
}

std::shared_ptr<SparseMatrix> INIFile::getSparseMatrix(const std::string& section, const std::string& tag) const
{
   THROWERROR_NOTIMPL();
}

void INIFile::put(const std::string& section, const std::string& tag, const Matrix &M)
{
   THROWERROR_NOTIMPL();
}

void INIFile::put(const std::string& section, const std::string& tag, const SparseMatrix &X)
{
   THROWERROR_NOTIMPL();
}

} //end namespace smurff