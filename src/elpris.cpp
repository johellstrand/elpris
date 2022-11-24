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
static int row;
static int col;
static int tableid;
#include <map>
#include <utility>

static std::map<int, std::map<int,std::pair<std::string,std::string> > > tables;

static void find_definitions(GumboNode *node)
{
    GumboAttribute *attr;
    if( col == 1 && node->type == GUMBO_NODE_TEXT )
    {
        auto &[k,v] = tables[tableid-1][row-1];
        k =  node->v.text.text;
        col++;
        return;
    }
    if( col == 2 && node->type == GUMBO_NODE_TEXT )
    {
        auto &[k,v] = tables[tableid-1][row-1];
        v =  node->v.text.text;
        col++;
        return;
    }
    
    
    if (node->type != GUMBO_NODE_ELEMENT)
    {
        return ;
    }
    
    if ( node->v.element.tag == GUMBO_TAG_TABLE ) {tableid++; row=col = 0; }

    if ( node->v.element.tag == GUMBO_TAG_TR ) {row++; col = 0; }

    if ( node->v.element.tag == GUMBO_TAG_TD && col == 0) col = 1;

    GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i)
    {
        find_definitions(static_cast<GumboNode *>(children->data[i]));
    }
    
    return ;
}

static std::map<int, std::map<int,std::pair<std::string,std::string> > >  scrape(std::string markup)
{
    std::string res = "entry";
    GumboOutput *output = gumbo_parse_with_options(&kGumboDefaultOptions, markup.data(), markup.length());
    
    find_definitions(output->root);
    
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    
    return tables;
}

static const char* ht = "<table class=\"table table-striped mb30\"><thead><tr><th>Månad</th> <th>Månadspris (SE3)</th></tr></thead><tbody><tr><td>November 2022 *</td><td>85,57 öre/kWh</td></tr><tr><td>Oktober 2022</td><td>80,65 öre/kWh</td></tr><tr><td>September 2022</td><td>228,63 öre/kWh</td></tr><tr><td>Augusti 2022</td><td>223,05 öre/kWh</td></tr><tr><td>Juli 2022</td><td>86,61 öre/kWh</td></tr><tr><td>Juni 2022</td><td>126,31 öre/kWh</td></tr><tr><td>Maj 2022</td><td>102,86 öre/kWh</td></tr><tr><td>April 2022</td><td>89,22 öre/kWh</td></tr><tr><td>Mars 2022</td><td>130,33 öre/kWh</td></tr><tr><td>Februari 2022</td><td>77,48 öre/kWh</td></tr><tr><td>Januari 2022</td><td>104,33 öre/kWh</td></tr><tr><td>December 2021</td><td>180,74 öre/kWh</td></tr><tr><td>November 2021</td><td>83,52 öre/kWh</td></tr></tbody></table>";
int main()
{
    // scrape( ht );
    // exit(1);
    auto html = request();
    auto t = std::move(scrape(html));
        
    for( const auto& [a,b] : t )
    {
        for( const auto& [k,v] : b )
        {
            const auto& [r,c] = v;
            printf("%2d %-25.25s %-25.25s\n", a, r.c_str(), c.c_str() );
//            std::cout << a << "-" << r << ":" << c << "\n";
        }
    }
}
