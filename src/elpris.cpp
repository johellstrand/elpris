#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include "curl/curl.h"

#define URL "https://www.elprisetjustnu.se/api/v1/prices/"

typedef size_t( * curl_write)(char * , size_t, size_t, std::string * );

static size_t cw( char * contents, size_t size, size_t nmemb, std::string * data )
{
    size_t new_size = size * nmemb;
    if( !data ) return 0;
    
    data->append(contents, new_size);
    return new_size;
}


static std::string request(std::string yyyy, std::string mm, std::string dd)
{
    CURLcode res_code = CURLE_FAILED_INIT;
    CURL * curl = curl_easy_init();
    std::string result;
    
    std::stringstream ss;
    
    ss << URL << yyyy << "/" << mm << "-" << dd << "_SE3.json";
    auto url = ss.str();
    curl_global_init(CURL_GLOBAL_ALL);
    
    if( curl )
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cw );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, & result);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        //        curl_easy_setopt(curl, CURLOPT_USERAGENT, "simple scraper");
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        res_code = curl_easy_perform(curl);
        
        if( res_code != CURLE_OK)
        {
            printf("%s\n", curl_easy_strerror(res_code));
            exit(EXIT_FAILURE);
        }
        //  curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    
    return result;
}

static std::string request(time_t t_c)
{
    auto lt = std::localtime(&t_c);
    std::stringstream yyyy;
    std::stringstream mm;
    std::stringstream dd;
    std::stringstream hh;
    
    yyyy << std::put_time( lt, "%Y");
    mm << std::put_time( lt, "%m");
    dd << std::put_time( lt, "%d");
    hh << std::put_time( lt, "%H");
    
    return request(yyyy.str(), mm.str(), dd.str());
}
#define TOKEN "SEK_per_kWh"
#include <map>
#include <vector>
static std::vector<int> parse( std::string s )
{
    //puts(s.c_str());
    std::vector<int> ret;
    auto p = strstr( s.c_str(), TOKEN );
    while( p )
    {
        
        std::string token;
        std::istringstream tokenStream(p);
        int i = 0;
        while (std::getline(tokenStream, token, ':'))
        {
            if( i )
            {
                
                ret.emplace_back(std::lround(100 * std::atof(token.c_str())));
                goto next;
            }
            i++;
        }
    next:
        p = strstr( p+strlen(TOKEN), TOKEN );
    }
    
    return ret;
}
int main()
{
    using namespace std::literals;
    
    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    
    std::map<time_t, std::vector<int>> vs;
    auto today = std::chrono::system_clock::to_time_t(now );
    auto tomorrow = std::chrono::system_clock::to_time_t(now + 24h);
    auto yesterday = std::chrono::system_clock::to_time_t(now - 24h);
    auto v = parse( request(today));
    if( v.size())
    {
        vs[today] = v;
        auto v = parse( request(tomorrow));
        if( v.size())
        {
            vs[tomorrow] = v;
        }
        /*
         v = parse( request(yesterday));
         if( v.size())
         {
         vs[yesterday] = v;
         }*/
    }
    
    char yyyymmdd[20];
    for( const auto& [d,v] : vs )
    {
        auto lt = std::localtime(&d);
        strftime(yyyymmdd, sizeof yyyymmdd, "%Y%m%d", lt );
        int tt = 0;
        for( const auto& j : v )
        {
            printf("%s %02d-%02d: %4d öre/kWh\n", yyyymmdd, tt, tt+1, j );
            tt++;
        }
    }
    
    exit(1);
}
