#include <string>
#include <iostream>
#include "curl/curl.h"
#include "gumbo.h"


typedef size_t( * curl_write)(char * , size_t, size_t, std::string * );

static size_t cw( char * contents, size_t size, size_t nmemb, std::string * data )
{
    size_t new_size = size * nmemb;
    if( !data ) return 0;
    
    data->append(contents, new_size);
    return new_size;
}


static std::string request()
{
    CURLcode res_code = CURLE_FAILED_INIT;
    CURL * curl = curl_easy_init();
    std::string result;
    std::string url = "https://www.elbruk.se/timpriser-se3-stockholm";
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    if( curl )
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cw );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, & result);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//        curl_easy_setopt(curl, CURLOPT_USERAGENT, "simple scraper");
        
        res_code = curl_easy_perform(curl);
        
        if( res_code != CURLE_OK) return curl_easy_strerror(res_code);
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    
    return result;
}


static std::string find_definitions(GumboNode *node)
{
  std::string res = "";
  GumboAttribute *attr;
  if (node->type != GUMBO_NODE_ELEMENT)
  {
    return res;
  }
 
  if ((attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
      strstr(attr->value, "dtText") != NULL)
  {
      res += "hej"; //extract_text(node);
    res += "\n";
  }
 
  GumboVector *children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i)
  {
    res += find_definitions(static_cast<GumboNode *>(children->data[i]));
  }
 
  return res;
}

static std::string scrape(std::string markup)
{
  std::string res = "entry";
  GumboOutput *output = gumbo_parse_with_options(&kGumboDefaultOptions, markup.data(), markup.length());
 
  res += find_definitions(output->root);
 
  gumbo_destroy_output(&kGumboDefaultOptions, output);
 
  return res;
}

int main()
{
    
    auto html = request();
    
    std::cout << scrape(html) << std::endl;
}
