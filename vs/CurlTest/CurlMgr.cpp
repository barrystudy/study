#include "CurlMgr.h"
#include <iostream>

CurlMgr::CurlMgr()
{
    m_taskNums = 0;
}

CurlMgr::~CurlMgr()
{
    m_running = false;
}

void CurlMgr::Start()
{
    m_running = true;
    m_pThread.reset(new boost::thread(&CurlMgr::ThreadFun, this));
}

void CurlMgr::Stop()
{
    m_running = false;
    if (m_pThread)
    {
        m_pThread->join();
        m_pThread.reset();
    }
}

void CurlMgr::Push(const CurlTask& task)
{
    boost::mutex::scoped_lock lock(m_mutex);
    m_queues.push(task);
    m_taskNums++;
}

size_t WriteData( void* buffer, size_t size, size_t count, void* stream )
{
    return fwrite((char*)buffer, size, count, (FILE*)stream);
}

void CurlMgr::ThreadFun()
{
    CURLM* curlm = curl_multi_init();
    while (m_running)
    {
        int currentTaskNums = 0;
        FILE* file = NULL;
        std::vector<CURL *> easyVec;
        {
            boost::mutex::scoped_lock lock(m_mutex);
            currentTaskNums = m_taskNums;
            while (!m_queues.empty())
            {
                const CurlTask& task = m_queues.top();
                file = fopen(task.m_filePath.c_str(), "ab+");
                CURL* easy = curl_easy_init();
                curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1);
                curl_easy_setopt(easy, CURLOPT_URL, task.m_url.c_str());
                curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, WriteData);
                curl_easy_setopt(easy, CURLOPT_WRITEDATA, file);
                curl_multi_add_handle(curlm, easy);
                easyVec.push_back(easy);
                m_queues.pop();
            }
        }
        int runingNums = 0;
        do 
        {
            CURLMcode ret = curl_multi_wait(curlm, NULL, 0, 2000, NULL);
            curl_multi_perform(curlm, &runingNums);
        } while (runingNums > currentTaskNums -1);

        int msgs_left = 0;
        CURLMsg* msg = NULL;
        while ((msg = curl_multi_info_read(curlm, &msgs_left)) != NULL)
        {
            if (msg->msg == CURLMSG_DONE) 
            {
                int retCode = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &retCode);
                // char* effective_url = NULL;
                //  curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &effective_url);
                //  fprintf(stdout, "url:%s status:%d %s\n", effective_url, http_status_code, curl_easy_strerror(msg->data.result));
                curl_multi_remove_handle(curlm, msg->easy_handle);
                //curl_easy_cleanup(msg->easy_handle);
            }
        }


//         for (size_t i = 0; i < easyVec.size(); ++i)
//         {
//             curl_multi_remove_handle(curlm, easyVec[i]);
//             curl_easy_cleanup(easyVec[i]);
//         }
        Sleep(1000);
    }
    curl_multi_cleanup(curlm);
}