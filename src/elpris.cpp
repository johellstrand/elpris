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
#include <map>
#include <vector>
#include <utility>

static std::map<int, std::map<int,std::vector<std::string> > > tables;

static void save_html_table( GumboNode *node, std::map<int,std::vector<std::string> >& table, int &row )
{
    int col = 0;
    GumboAttribute *attr;
    if(  node->type == GUMBO_NODE_TEXT )
    {
        table[row-1].push_back(node->v.text.text );
        return;
    }
    
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if ( node->v.element.tag == GUMBO_TAG_TR ) {row++; col = 0; }
    
    if ( node->v.element.tag == GUMBO_TAG_TD && col == 0) col = 1;
    
    GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i)
    {
        save_html_table(static_cast<GumboNode *>(children->data[i]), table, row );
    }
}

static void save_html_tables( GumboNode *node )
{
    int row = 0;
    static int tableid=0;

    if( node->type != GUMBO_NODE_ELEMENT ) return;

    if ( node->v.element.tag == GUMBO_TAG_TABLE ) { save_html_table( node, tables[tableid++], row );  }
    
    GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i)
    {
        save_html_tables(static_cast<GumboNode *>(children->data[i]));
    }
    return ;
}

#include <fstream>
static std::map<int, std::map<int,std::vector<std::string> > > scrape(std::string markup)
{
    GumboOutput *output = gumbo_parse_with_options(&kGumboDefaultOptions, markup.data(), markup.length());
    
    save_html_tables(output->root);
    
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    
    return tables;
}

#include "ReadOnlyFileMMap.h"
#define FILECACHE_DIR "cache"
#define FILECACHE "elpris_%s.html"
#include <chrono>
#include <format>
#include <sstream>

int main()
{
    std::time_t t2 = std::time(nullptr);
    std::tm tm = *std::localtime(&t2);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y%m%d_%H");

    char filename[100];
    std::filesystem::create_directory(FILECACHE_DIR);
    char filename_format[100];

    snprintf( filename_format, sizeof filename_format, "%s%c%s", FILECACHE_DIR, std::filesystem::path::preferred_separator, FILECACHE );
    snprintf( filename, sizeof filename, filename_format, ss.str().c_str() );
    
    ReadOnlyFileMMap map(filename);

    std::string html;
    if( map.open() )
    {
        std::cout << "From cache: " << filename << "\n";
        html.assign( (const char*)map.const_data(), map.size() );
    }
    else
    {
        html = request();
        std::ofstream myfile;
        myfile.open (filename);
        myfile << html;
        myfile.close();
    }
    
    
    auto t = std::move(scrape(html));
    
    for( const auto& [a,b] : t )
    {
        for( const auto& [k,v] : b )
        {
            printf("%2d ", a );
            for( const auto& e : v )
            {
                printf("%-25.25s", e.c_str() );
            }
            puts("");
        }
    }
}
