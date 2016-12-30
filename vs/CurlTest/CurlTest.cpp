#include <iostream>
#include <curl/curl.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <time.h>
#include "CurlMgr.h"
//http://cn.engadget.com/rss.xml


size_t write_data(void* buffer, size_t size, size_t count, void* stream) {
    return size * count;
    //return fwrite((char*)buffer, size, count, (FILE*)stream);
}

void curl_multi_demo() {
    CURLM* curlm = curl_multi_init();
    //FILE* f = fopen("hello.jpg", "ab+");
    CURL* easy_handle = curl_easy_init();
    curl_easy_setopt(easy_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(easy_handle, CURLOPT_URL, "http://o.aolcdn.com/hss/storage/midas/6ed0a1e627e5fe685e43832d5f60b559/204750905/tesla-model-x-accident-prediction-ed.jpg");
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
    //curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, f);
    curl_multi_add_handle(curlm, easy_handle);

    int running_handlers = 0;
    do {
        curl_multi_wait(curlm, NULL, 0, 2000, NULL);
        curl_multi_perform(curlm, &running_handlers);
    } while (running_handlers > 0);
//     if (f)
//     {
//        fclose(f);
//        f = NULL;
//     }
    int msgs_left = 0;
    CURLMsg* msg = NULL;
    while ((msg = curl_multi_info_read(curlm, &msgs_left)) != NULL) {
        if (msg->msg == CURLMSG_DONE) {
            int http_status_code = 0;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
            char* effective_url = NULL;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &effective_url);
            fprintf(stdout, "url:%s status:%d %s\n", effective_url, http_status_code, curl_easy_strerror(msg->data.result));
            curl_multi_remove_handle(curlm, msg->easy_handle);
            //curl_easy_cleanup(msg->easy_handle);
        }
    }
    curl_multi_cleanup(curlm);
}

int main()
{
    //curl_multi_demo();
    std::string url("http://www.chong4.com.cn/img01/linshi/2016-12-24/943c472b0d8a8ba0b2fb5fb1e28d6390.jpg");
    std::string url2("http://www.chong4.com.cn/img01/linshi/2016-12-24/b7e77c5b494d1d2179ee4cbf09b793d2.jpg");
    CurlMgr mgr;
    mgr.Start();
    mgr.Push(CurlTask(url, "./hello.jpg", 1));
    Sleep(3000);
    mgr.Push(CurlTask(url2, "./hello2.jpg", 2));
    Sleep(10000);
    mgr.Stop();
    system("pause");
}