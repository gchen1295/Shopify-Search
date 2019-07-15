#include "searcher.h"

void unique(std::vector<std::string> &keywords)
{
  std::vector<std::string>::iterator it;
  std::sort(keywords.begin(), keywords.end());
  it = std::unique(keywords.begin(), keywords.end());
  keywords.resize(std::distance(keywords.begin(), it));
}

void split(std::string sentence, std::string delimiter, std::vector<std::string> &words)
{
  size_t dpos = 0;
  std::string tmp;
  sentence += delimiter;
  while ((dpos = sentence.find(delimiter)) != std::string::npos)
  {
    tmp = sentence.substr(0, dpos);
    words.push_back(tmp);
    sentence.erase(0, dpos + delimiter.length());
  }
}

Found Searcher::findByJSON(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound)
{
  try
  {
    std::string url = "https://" + domain + "/products.json";
    std::list<std::string> header;

    header.push_back("user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
    cURLpp::Cleanup clean;
    cURLpp::Easy r;
    r.setOpt(new cURLpp::options::Url(url));
    r.setOpt(new cURLpp::options::HttpHeader(header));
    std::ostringstream res;
    r.setOpt(new cURLpp::options::WriteStream(&res));
    r.perform();
    long status_code = cURLpp::infos::ResponseCode::get(r);
    std::string redirect_url = curlpp::infos::EffectiveUrl::get(r);
    if (status_code >= 400)
    {
      //std::cout << "Page returned a " << std::to_string(status_code) << " error\n";
      Found fp = Found();
      fp.error = "Error: " + std::to_string(status_code);
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else if (status_code == 0)
    {
      Found fp = Found();
      fp.error = "Page not found";
      return fp;
    }
    else if (redirect_url.find("password") != std::string::npos)
    {
      Found fp = Found();
      fp.error = "Password page found!";
      return fp;
    }
    else
    {
      std::vector<Found> foundProducts;
      std::vector<Found> foundProducts2;
      int i = 0;
      std::vector<std::string>::iterator itr;
      //std::vector<std::string> foundtitle;
      //std::vector<std::string> foundhandle;
      unique(keywords);

      rapidjson::Document doc;
      doc.Parse(res.str().c_str());
      const rapidjson::Value &p = doc["products"];

      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        for (rapidjson::SizeType i = 0; i < p.Size(); ++i)
        {
          std::string kw = *itr;
          std::string title = p[i]["title"].GetString();
          std::string handle = p[i]["handle"].GetString();
          std::string askSize = size;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          std::transform(handle.begin(), handle.end(), handle.begin(), ::tolower);
          std::transform(askSize.begin(), askSize.end(), askSize.begin(), ::tolower);

          if (title.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = title;
            fp.productHandle = p[i]["handle"].GetString();
            fp.productURL = "https://" + domain + "/products/" + fp.productHandle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";
            if (askSize == "random" || askSize == "any")
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            else
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);
                if (vtitle.find(askSize) != std::string::npos)
                {
                  fp.size = vtitle;
                  fp.variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            if (fp.size == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            //foundtitle.push_back(title);
            //foundhandle.push_back(handle);
            foundProducts.push_back(fp);
          }
          if (handle.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = title;
            fp.productHandle = p[i]["handle"].GetString();
            fp.productURL = "https://" + domain + "/products/" + fp.productHandle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";

            if (askSize == "random" || askSize == "any")
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            else
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);
                if (vtitle.find(askSize) != std::string::npos)
                {
                  fp.size = vtitle;
                  fp.variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            if (fp.size == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            //foundtitle.push_back(title);
            //foundhandle.push_back(handle);
            foundProducts2.push_back(fp);
          }
        }
      }
      //unique(foundtitle);
      //unique(foundhandle);
      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string title = itr2->productTitle;
          if (title.find(kw) == std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string title = itr2->productTitle;
          if (title.find(kw) != std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (foundProducts.size() == 0)
      {
        for (itr = keywords.begin(); itr != keywords.end(); ++itr)
        {
          std::string kw = *itr;
          auto itr2 = foundProducts2.begin();
          while (itr2 != foundProducts2.end())
          {
            std::string handle = itr2->productHandle;
            if (handle.find(kw) == std::string::npos)
            {
              itr2 = foundProducts2.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
        {
          std::string kw = *itr;
          auto itr2 = foundProducts2.begin();
          while (itr2 != foundProducts2.end())
          {
            std::string title = itr2->productHandle;
            if (title.find(kw) != std::string::npos)
            {
              itr2 = foundProducts2.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
      }
      if (foundProducts.size() > 0)
      {
        return foundProducts.at(0);
      }
      else if (foundProducts2.size() > 0)
      {
        return foundProducts2.at(0);
      }
      else
      {
        Found fp = Found();
        fp.error = "No product found";
        return fp;
      }
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp = Found();
    fp.error = e.what();
    return fp;
  }
}

Found Searcher::findByJSON(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound)
{
  try
  {
    std::string url = "https://" + domain + "/products.json";
    std::list<std::string> header;
    std::vector<std::string> proxyParts;
    split(proxy, ":", proxyParts);

    header.push_back("user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
    cURLpp::Cleanup clean;
    cURLpp::Easy r;
    r.setOpt(new cURLpp::options::Url(url));
    r.setOpt(new cURLpp::options::Proxy(proxyParts[0] + ":" + proxyParts[1]));
    r.setOpt(new cURLpp::options::ProxyUserPwd(proxyParts[2] + ":" + proxyParts[3]));
    r.setOpt(new cURLpp::options::HttpHeader(header));
    std::ostringstream res;
    r.setOpt(new cURLpp::options::WriteStream(&res));
    r.perform();
    long status_code = cURLpp::infos::ResponseCode::get(r);
    std::string redirect_url = curlpp::infos::EffectiveUrl::get(r);
    if (status_code >= 400)
    {
      //std::cout << "Page returned a " << std::to_string(status_code) << " error\n";
      Found fp = Found();
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else if (status_code == 0)
    {
      Found fp = Found();
      fp.error = "Page not found";
      return fp;
    }
    else if (redirect_url.find("password") != std::string::npos)
    {
      Found fp = Found();
      fp.error = "Password page found!";
      return fp;
    }
    else
    {
      std::vector<Found> foundProducts;
      std::vector<Found> foundProducts2;
      int i = 0;
      std::vector<std::string>::iterator itr;
      //std::vector<std::string> foundtitle;
      //std::vector<std::string> foundhandle;
      unique(keywords);

      rapidjson::Document doc;
      doc.Parse(res.str().c_str());
      const rapidjson::Value &p = doc["products"];

      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        for (rapidjson::SizeType i = 0; i < p.Size(); ++i)
        {
          std::string kw = *itr;
          std::string title = p[i]["title"].GetString();
          std::string handle = p[i]["handle"].GetString();
          std::string askSize = size;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          std::transform(handle.begin(), handle.end(), handle.begin(), ::tolower);
          std::transform(askSize.begin(), askSize.end(), askSize.begin(), ::tolower);

          if (title.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = title;
            fp.productHandle = p[i]["handle"].GetString();
            fp.productURL = "https://" + domain + "/products/" + fp.productHandle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";
            if (askSize == "random" || askSize == "any")
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            else
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);
                if (vtitle.find(askSize) != std::string::npos)
                {
                  fp.size = vtitle;
                  fp.variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            if (fp.size == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            //foundtitle.push_back(title);
            //foundhandle.push_back(handle);
            foundProducts.push_back(fp);
          }
          if (handle.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = title;
            fp.productHandle = p[i]["handle"].GetString();
            fp.productURL = "https://" + domain + "/products/" + fp.productHandle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";

            if (askSize == "random" || askSize == "any")
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            else
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);
                if (vtitle.find(askSize) != std::string::npos)
                {
                  fp.size = vtitle;
                  fp.variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            if (fp.size == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              fp.size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              fp.variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
            //foundtitle.push_back(title);
            //foundhandle.push_back(handle);
            foundProducts2.push_back(fp);
          }
        }
      }
      //unique(foundtitle);
      //unique(foundhandle);
      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string title = itr2->productTitle;
          if (title.find(kw) == std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string title = itr2->productTitle;
          if (title.find(kw) != std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (foundProducts.size() == 0)
      {
        for (itr = keywords.begin(); itr != keywords.end(); ++itr)
        {
          std::string kw = *itr;
          auto itr2 = foundProducts2.begin();
          while (itr2 != foundProducts2.end())
          {
            std::string handle = itr2->productHandle;
            if (handle.find(kw) == std::string::npos)
            {
              itr2 = foundProducts2.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
        {
          std::string kw = *itr;
          auto itr2 = foundProducts2.begin();
          while (itr2 != foundProducts2.end())
          {
            std::string title = itr2->productHandle;
            if (title.find(kw) != std::string::npos)
            {
              itr2 = foundProducts2.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
      }
      if (foundProducts.size() > 0)
      {
        return foundProducts.at(0);
      }
      else if (foundProducts2.size() > 0)
      {
        return foundProducts2.at(0);
      }
      else
      {
        Found fp = Found();
        fp.error = "No product found";
        return fp;
      }
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp = Found();
    fp.error = e.what();
    return fp;
  }
}

Found Searcher::findByHandle(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size)
{
  try
  {
    std::string url = "https://" + domain + "/products.json";
    std::list<std::string> header;

    header.push_back("user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
    cURLpp::Cleanup clean;
    cURLpp::Easy r;
    r.setOpt(new cURLpp::options::Url(url));
    r.setOpt(new cURLpp::options::HttpHeader(header));
    std::ostringstream res;
    r.setOpt(new cURLpp::options::WriteStream(&res));
    r.perform();
    long status_code = cURLpp::infos::ResponseCode::get(r);
    std::string redirect_url = curlpp::infos::EffectiveUrl::get(r);
    if (status_code >= 400)
    {
      //std::cout << "Page returned a " << std::to_string(status_code) << " error\n";
      Found fp = Found();
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else if (status_code == 0)
    {
      Found fp = Found();
      fp.error = "Page not found";
      return fp;
    }
    else if (redirect_url.find("password") != std::string::npos)
    {
      Found fp = Found();
      fp.error = "Password page found!";
      return fp;
    }
    else
    {
      std::vector<Found> foundProducts;
      int i = 0;
      std::vector<std::string>::iterator itr;
      std::vector<std::string> foundhandle;
      unique(keywords);

      rapidjson::Document doc;
      doc.Parse(res.str().c_str());
      const rapidjson::Value &p = doc["products"];

      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        for (rapidjson::SizeType i = 0; i < p.Size(); ++i)
        {
          std::string kw = *itr;
          std::string handle = p[i]["handle"].GetString();
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          std::transform(handle.begin(), handle.end(), handle.begin(), ::tolower);

          if (handle.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = p[i]["title"].GetString();
            fp.productHandle = handle;
            fp.productURL = "https://" + domain + "/products/" + handle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";

            for (rapidjson::SizeType i = 0; i < var.Size(); i++)
            {
              std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
              if (vtitle.find(size) != std::string::npos)
              {
                fp.size = vtitle;
                fp.variantID = std::to_string(var[i]["id"].GetInt64());
              }
            }
            foundhandle.push_back(handle);
            foundProducts.push_back(fp);
          }
        }
      }
      unique(foundhandle);
      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string handle = itr2->productHandle;
          if (handle.find(kw) == std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string handle = itr2->productHandle;
          if (handle.find(kw) != std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (foundProducts.size() != 0)
      {
        return foundProducts.at(0);
      }
      else
      {
        Found fp = Found();
        fp.error = "No product found";
        return fp;
      }
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp = Found();
    fp.error = e.what();
    return fp;
  }
}

Found Searcher::findByHandle(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size)
{
  try
  {
    std::string url = "https://" + domain + "/products.json";
    std::list<std::string> header;
    std::vector<std::string> proxyParts;
    split(proxy, ":", proxyParts);

    header.push_back("user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36");
    cURLpp::Cleanup clean;
    cURLpp::Easy r;
    r.setOpt(new cURLpp::options::Url(url));
    r.setOpt(new cURLpp::options::Proxy(proxyParts[0] + ":" + proxyParts[1]));
    r.setOpt(new cURLpp::options::ProxyUserPwd(proxyParts[2] + ":" + proxyParts[3]));
    r.setOpt(new cURLpp::options::HttpHeader(header));
    std::ostringstream res;
    r.setOpt(new cURLpp::options::WriteStream(&res));
    r.perform();
    long status_code = cURLpp::infos::ResponseCode::get(r);
    std::string redirect_url = curlpp::infos::EffectiveUrl::get(r);
    if (status_code >= 400)
    {
      //std::cout << "Page returned a " << std::to_string(status_code) << " error\n";
      Found fp = Found();
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else if (status_code == 0)
    {
      Found fp = Found();
      fp.error = "Page not found";
      return fp;
    }
    else if (redirect_url.find("password") != std::string::npos)
    {
      Found fp = Found();
      fp.error = "Password page found!";
      return fp;
    }
    else
    {
      std::vector<Found> foundProducts;
      int i = 0;
      std::vector<std::string>::iterator itr;
      std::vector<std::string> foundhandle;
      unique(keywords);

      rapidjson::Document doc;
      doc.Parse(res.str().c_str());
      const rapidjson::Value &p = doc["products"];

      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        for (rapidjson::SizeType i = 0; i < p.Size(); ++i)
        {
          std::string kw = *itr;
          std::string handle = p[i]["handle"].GetString();
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          std::transform(handle.begin(), handle.end(), handle.begin(), ::tolower);

          if (handle.find(kw) != std::string::npos)
          {
            //std::cout << "Found in title: " << title << std::endl;
            //std::cout << "Matched keyword: " << kw << std::endl;
            Found fp = Found();
            //BUILD OUR PRODUCT
            fp.productTitle = p[i]["title"].GetString();
            fp.productHandle = handle;
            fp.productURL = "https://" + domain + "/products/" + handle;
            fp.productImage = p[i]["images"].Capacity() > 0 ? p[i]["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["products"][i]["variants"];
            fp.productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";

            for (rapidjson::SizeType i = 0; i < var.Size(); i++)
            {
              std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
              if (vtitle.find(size) != std::string::npos)
              {
                fp.size = vtitle;
                fp.variantID = std::to_string(var[i]["id"].GetInt64());
              }
            }
            foundhandle.push_back(handle);
            foundProducts.push_back(fp);
          }
        }
      }
      unique(foundhandle);
      for (itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string handle = itr2->productHandle;
          if (handle.find(kw) == std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        auto itr2 = foundProducts.begin();
        while (itr2 != foundProducts.end())
        {
          std::string handle = itr2->productHandle;
          if (handle.find(kw) != std::string::npos)
          {
            itr2 = foundProducts.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (foundProducts.size() != 0)
      {
        return foundProducts.at(0);
      }
      else
      {
        Found fp = Found();
        fp.error = "No product found";
        return fp;
      }
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp = Found();
    fp.error = e.what();
    return fp;
  }
}

Found Searcher::findBySiteMap(std::string domain, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound)
{
  try
  {
    std::string url = "https://" + domain + "/sitemap.xml";
    std::ostringstream p1res;
    long status_code;
    std::list<std::string> header;

    header.push_back("Accept: application/xml");
    header.push_back("charset: utf-8");

    curlpp::Cleanup clean;
    curlpp::Easy r;

    r.setOpt(curlpp::options::UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36"));
    r.setOpt(curlpp::options::HttpHeader(header));
    r.setOpt(curlpp::options::Url(url));
    r.setOpt(curlpp::options::WriteStream(&p1res));
    r.perform();
    status_code = curlpp::Infos::ResponseCode::get(r);
    std::string redirect_url = curlpp::infos::EffectiveUrl::get(r);
    if (status_code == 0)
    {
      Found fp;
      fp.error = "Page not found";
      return fp;
    }
    else if (status_code >= 400)
    {
      Found fp;
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else if (redirect_url.find("password") != std::string::npos)
    {
      Found fp = Found();
      fp.error = "Password page found!";
      return fp;
    }
    else
    {
      std::string p1s = p1res.str();
      char *p1resdoc = new char[p1s.size() + 1];
      strcpy(p1resdoc, p1s.c_str());

      rapidxml::xml_document<> doc1;
      rapidxml::xml_node<> *root_node;

      doc1.parse<0>(p1resdoc);
      root_node = doc1.first_node("sitemapindex");

      std::list<std::string> productListings;
      std::list<Found> productLists;
      for (rapidxml::xml_node<> *sitemapnode = root_node->first_node("sitemap"); sitemapnode; sitemapnode = sitemapnode->next_sibling())
      {
        rapidxml::xml_node<> *locnode = sitemapnode->first_node("loc");
        std::string val = locnode->value();
        if (val.find("products") != std::string::npos)
        {
          productListings.push_back(val);
        }
      }
      delete[] p1resdoc;
      for (auto &m : productListings)
      {
        url = m;
        p1res.str("");
        p1res.clear();
        r.setOpt(curlpp::options::Url(url));
        r.setOpt(curlpp::options::WriteStream(&p1res));
        r.perform();
        status_code = curlpp::Infos::ResponseCode::get(r);

        if (status_code == 0)
        {
          Found fp;
          fp.error = "Page not found";
          return fp;
        }
        else if (status_code >= 400)
        {
          Found fp;
          fp.error = "Page returned a " + std::to_string(status_code) + " error";
          return fp;
        }
        else
        {
          doc1.clear();
          p1s.clear();
          p1s = p1res.str();
          char *resdoc = new char[p1s.size() + 1];
          strcpy(resdoc, p1s.c_str());
          doc1.parse<0>(resdoc);

          root_node = doc1.first_node("urlset");
          for (rapidxml::xml_node<> *urlnode = root_node->first_node("url"); urlnode; urlnode = urlnode->next_sibling())
          {
            rapidxml::xml_node<> *locnode = urlnode->first_node("loc");
            std::string prodUrl = locnode->value();
            if (prodUrl.find("/products/") != std::string::npos)
            {
              rapidxml::xml_node<> *imgnode = urlnode->first_node("image:image");
              Found tmp = Found();
              tmp.productURL = prodUrl;
              if (imgnode != 0)
              {
                rapidxml::xml_node<> *titlenode = imgnode->first_node("image:title");
                rapidxml::xml_node<> *imgURLnode = imgnode->first_node("image:loc");
                tmp.productTitle = titlenode->value();
                tmp.productImage = imgURLnode->value();
              }
              productLists.push_back(tmp);
            }
            //std::cout << "Finished parsing sitemap." << std::endl;
          }
          //std::cout << "Filtering products" << std::endl;
          delete[] resdoc;
        }
      }

      std::vector<std::string>::iterator itr;
      std::list<Found> prodtmp = productLists;
      for (auto itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        auto itr2 = productLists.begin();
        while (itr2 != productLists.end())
        {
          std::string title = itr2->productTitle;
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          if (title.find(kw) == std::string::npos)
          {
            itr2 = productLists.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (auto itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        auto itr2 = productLists.begin();
        while (itr2 != productLists.end())
        {
          std::string title = itr2->productTitle;
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          if (title.find(kw) != std::string::npos)
          {
            itr2 = productLists.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (productLists.size() == 0)
      {
        for (auto itr = keywords.begin(); itr != keywords.end(); ++itr)
        {
          std::string kw = *itr;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          auto itr2 = prodtmp.begin();
          while (itr2 != prodtmp.end())
          {
            std::string title = itr2->productURL;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(kw) == std::string::npos)
            {
              
              itr2 = prodtmp.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        for (auto itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
        {
          std::string kw = *itr;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          auto itr2 = prodtmp.begin();
          while (itr2 != prodtmp.end())
          {
            std::string title = itr2->productURL;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(kw) != std::string::npos)
            {
              itr2 = prodtmp.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        if (prodtmp.size() > 0)
        {
          productLists.push_back(prodtmp.front());
        }
      }
      if (productLists.size() > 0)
      {

        curlpp::Easy r2;
        url = productLists.front().productURL + ".json";
        header.clear();
        header.push_back("Accept: application/json");
        header.push_back("charset: utf-8");
        p1res.str("");
        p1res.clear();
        r2.setOpt(curlpp::options::UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36"));
        r2.setOpt(curlpp::options::HttpHeader(header));
        r2.setOpt(curlpp::options::Url(url));
        r2.setOpt(curlpp::options::WriteStream(&p1res));
        r2.perform();
        status_code = curlpp::Infos::ResponseCode::get(r2);

        if (status_code == 0)
        {
          Found fp;
          fp.error = "Page not found";
          return fp;
        }
        else if (status_code >= 400)
        {
          Found fp;
          fp.error = "Product page returned a " + std::to_string(status_code) + " error";
          return fp;
        }
        else
        {
          rapidjson::Document doc;
          std::string prodres = p1res.str();
          doc.Parse(prodres.c_str());
          if (doc.IsObject())
          {
            const rapidjson::Value &p = doc["product"];

            std::string handle = p["handle"].GetString();

            Found fp = Found();
            //BUILD OUR PRODUCT
            productLists.front().productTitle = p["title"].IsNull() ? "" : p["title"].GetString();
            productLists.front().productHandle = handle;
            productLists.front().productURL = "https://" + domain + "/products/" + handle;
            productLists.front().productImage = p["images"].Capacity() > 0 ? p["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["product"]["variants"];
            productLists.front().productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";
            std::string sz = size;
            std::transform(sz.begin(), sz.end(), sz.begin(), ::tolower);
            if (sz != "random" && sz != "any")
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);
                if (vtitle.find(sz) != std::string::npos)
                {
                  productLists.front().size = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                  productLists.front().variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            else
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              productLists.front().size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              productLists.front().variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }

            if (productLists.front().variantID == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              productLists.front().size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              productLists.front().variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
          }
          else
          {
            productLists.front().error = "Product not available";
          }
          return productLists.front();
        }
      }
      Found fp;
      fp.error = "Product not found";
      return fp;
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp;
    fp.error = e.what();
    return fp;
  }
}

Found Searcher::findBySiteMap(std::string domain, std::string proxy, std::vector<std::string> keywords, std::vector<std::string> negKeywords, std::string size, bool randomNotFound)
{
  try
  {
    std::string url = "https://" + domain + "/sitemap.xml";
    std::ostringstream p1res;
    long status_code;
    std::list<std::string> header;
    std::vector<std::string> proxyParts;
    split(proxy, ":", proxyParts);

    header.push_back("Accept: application/xml");
    header.push_back("charset: utf-8");

    curlpp::Cleanup clean;
    curlpp::Easy r;

    r.setOpt(curlpp::options::UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36"));
    r.setOpt(new cURLpp::options::Proxy(proxyParts[0] + ":" + proxyParts[1]));
    r.setOpt(new cURLpp::options::ProxyUserPwd(proxyParts[2] + ":" + proxyParts[3]));
    r.setOpt(curlpp::options::HttpHeader(header));
    r.setOpt(curlpp::options::Url(url));
    r.setOpt(curlpp::options::WriteStream(&p1res));
    r.perform();
    status_code = curlpp::Infos::ResponseCode::get(r);

    if (status_code == 0)
    {
      Found fp;
      fp.error = "Page not found";
      return fp;
    }
    else if (status_code >= 400)
    {
      Found fp;
      if (status_code == 401)
      {
        fp.error = "Password page found!";
      }
      else
      {
        fp.error = "Page returned a " + std::to_string(status_code) + " error";
      }
      return fp;
    }
    else
    {
      std::string p1s = p1res.str();
      char *p1resdoc = new char[p1s.size() + 1];
      strcpy(p1resdoc, p1s.c_str());

      rapidxml::xml_document<> doc1;
      rapidxml::xml_node<> *root_node;

      doc1.parse<0>(p1resdoc);
      root_node = doc1.first_node("sitemapindex");

      std::list<std::string> productListings;
      std::list<Found> productLists;
      for (rapidxml::xml_node<> *sitemapnode = root_node->first_node("sitemap"); sitemapnode; sitemapnode = sitemapnode->next_sibling())
      {
        rapidxml::xml_node<> *locnode = sitemapnode->first_node("loc");
        std::string val = locnode->value();
        if (val.find("products") != std::string::npos)
        {
          productListings.push_back(val);
        }
      }
      delete[] p1resdoc;
      for (auto &m : productListings)
      {
        url = m;
        p1res.str("");
        p1res.clear();
        r.setOpt(curlpp::options::Url(url));
        r.setOpt(curlpp::options::WriteStream(&p1res));
        r.perform();
        status_code = curlpp::Infos::ResponseCode::get(r);

        if (status_code == 0)
        {
          Found fp;
          fp.error = "Page not found";
          return fp;
        }
        else if (status_code >= 400)
        {
          Found fp;
          fp.error = "Page returned a " + std::to_string(status_code) + " error";
          return fp;
        }
        else
        {
          doc1.clear();
          p1s.clear();
          p1s = p1res.str();
          char *resdoc = new char[p1s.size() + 1];
          strcpy(resdoc, p1s.c_str());
          doc1.parse<0>(resdoc);

          root_node = doc1.first_node("urlset");
          for (rapidxml::xml_node<> *urlnode = root_node->first_node("url"); urlnode; urlnode = urlnode->next_sibling())
          {
            rapidxml::xml_node<> *locnode = urlnode->first_node("loc");
            std::string prodUrl = locnode->value();
            if (prodUrl.find("/products/") != std::string::npos)
            {
              rapidxml::xml_node<> *imgnode = urlnode->first_node("image:image");
              Found tmp = Found();
              tmp.productURL = prodUrl;
              if (imgnode != 0)
              {
                rapidxml::xml_node<> *titlenode = imgnode->first_node("image:title");
                rapidxml::xml_node<> *imgURLnode = imgnode->first_node("image:loc");
                tmp.productTitle = titlenode->value();
                tmp.productImage = imgURLnode->value();
              }
              productLists.push_back(tmp);
            }
            //std::cout << "Finished parsing sitemap." << std::endl;
          }
          //std::cout << "Filtering products" << std::endl;
          delete[] resdoc;
        }
      }

      std::vector<std::string>::iterator itr;
      std::list<Found> prodtmp = productLists;
      for (auto itr = keywords.begin(); itr != keywords.end(); ++itr)
      {
        std::string kw = *itr;
        std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        auto itr2 = productLists.begin();
        while (itr2 != productLists.end())
        {
          std::string title = itr2->productTitle;
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          if (title.find(kw) == std::string::npos)
          {
            itr2 = productLists.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      for (auto itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
      {
        std::string kw = *itr;
        std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        auto itr2 = productLists.begin();
        while (itr2 != productLists.end())
        {
          std::string title = itr2->productTitle;
          std::transform(title.begin(), title.end(), title.begin(), ::tolower);
          if (title.find(kw) != std::string::npos)
          {
            itr2 = productLists.erase(itr2);
          }
          else
          {
            ++itr2;
          }
        }
      }
      if (productLists.size() == 0)
      {
        for (auto itr = keywords.begin(); itr != keywords.end(); ++itr)
        {
          std::string kw = *itr;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          auto itr2 = prodtmp.begin();
          while (itr2 != prodtmp.end())
          {
            std::string title = itr2->productURL;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(kw) == std::string::npos)
            {
              itr2 = prodtmp.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        for (auto itr = negKeywords.begin(); itr != negKeywords.end(); ++itr)
        {
          std::string kw = *itr;
          std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
          auto itr2 = prodtmp.begin();
          while (itr2 != prodtmp.end())
          {
            std::string title = itr2->productURL;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(kw) != std::string::npos)
            {
              itr2 = prodtmp.erase(itr2);
            }
            else
            {
              ++itr2;
            }
          }
        }
        if (prodtmp.size() > 0)
        {
          productLists.push_back(prodtmp.front());
        }
      }

      if (productLists.size() > 0)
      {
        curlpp::Easy r2;
        url = productLists.front().productURL + ".json";
        header.clear();
        header.push_back("Accept: application/json");
        header.push_back("charset: utf-8");
        p1res.str("");
        p1res.clear();
        r2.setOpt(curlpp::options::UserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36"));
        r2.setOpt(curlpp::options::HttpHeader(header));
        r2.setOpt(curlpp::options::Url(url));
        r2.setOpt(curlpp::options::WriteStream(&p1res));
        r2.perform();
        status_code = curlpp::Infos::ResponseCode::get(r2);

        if (status_code == 0)
        {
          Found fp;
          fp.error = "Page not found";
          return fp;
        }
        else if (status_code >= 400)
        {
          Found fp;
          fp.error = "Product page returned a " + std::to_string(status_code) + " error";
          return fp;
        }
        else
        {
          rapidjson::Document doc;
          std::string prodres = p1res.str();
          doc.Parse(prodres.c_str());
          if (doc.IsObject())
          {
            const rapidjson::Value &p = doc["product"];

            std::string handle = p["handle"].GetString();

            Found fp = Found();
            //BUILD OUR PRODUCT
            productLists.front().productTitle = p["title"].IsNull() ? "" : p["title"].GetString();
            productLists.front().productHandle = handle;
            productLists.front().productURL = "https://" + domain + "/products/" + handle;
            productLists.front().productImage = p["images"].Capacity() > 0 ? p["images"][0]["src"].GetString() : "";

            // Loop through our variants
            const rapidjson::Value &var = doc["product"]["variants"];
            productLists.front().productPrice = var.Size() > 0 ? var[0]["price"].GetString() : "";
            std::string sz = size;
            std::transform(sz.begin(), sz.end(), sz.begin(), ::tolower);
            if (sz != "any" && sz != "random")
            {
              for (rapidjson::SizeType i = 0; i < var.Size(); i++)
              {
                std::string vtitle = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                std::transform(vtitle.begin(), vtitle.end(), vtitle.begin(), ::tolower);

                if (vtitle.find(sz) != std::string::npos)
                {
                  productLists.front().size = var[i]["title"].IsNull() ? "" : var[i]["title"].GetString();
                  productLists.front().variantID = std::to_string(var[i]["id"].GetInt64());
                }
              }
            }
            else
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              productLists.front().size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              productLists.front().variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }

            if (productLists.front().variantID == "" && randomNotFound)
            {
              std::srand(time(NULL));
              int randomInt = std::rand() % var.Size();
              productLists.front().size = var[randomInt]["title"].IsNull() ? "" : var[randomInt]["title"].GetString();
              productLists.front().variantID = std::to_string(var[randomInt]["id"].GetInt64());
            }
          }
          else
          {
            productLists.front().error = "Product not available";
          }
          return productLists.front();
        }
      }
      Found fp;
      fp.error = "Product not found";
      return fp;
    }
  }
  catch (curlpp::RuntimeError &e)
  {
    Found fp;
    fp.error = e.what();
    return fp;
  }
}
