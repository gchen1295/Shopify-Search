#include "rapidjson/rapidjson.h"
#include "rapidjson/pointer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <sstream>
#include <rapidxml/rapidxml.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>

struct Found
{
  std::string productTitle;
  std::string productHandle;
  std::string productImage;
  std::string productPrice;
  std::string productURL;
  std::string variantID = "";
  std::string size = "";
  std::string error = "";
};

namespace Searcher
{
Found findByJSON(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound);
Found findByJSON(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound);
Found findByHandle(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size);
Found findByHandle(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size);
Found findBySiteMap(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound);
Found findBySiteMap(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound);
} // namespace Searcher