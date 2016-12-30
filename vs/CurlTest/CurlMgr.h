#pragma once

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/make_shared.hpp>
#include <queue>
#include <curl/curl.h>

struct CurlTask
{
    CurlTask(const std::string& url, const std::string& filePath, int priority): m_url(url), m_filePath(filePath), m_priority(priority) {}
    friend bool operator<(const CurlTask& lhs, const CurlTask& rhs)
    {
        return lhs.m_priority < rhs.m_priority;
    }
    std::string m_url;
    std::string m_filePath;
    int m_priority;
};

class CurlMgr
{
public:
    CurlMgr();
    ~CurlMgr();
    void Start();
    void Stop();
    void Push(const CurlTask& task);
private:
    void ThreadFun();
private:
    boost::shared_ptr<boost::thread> m_pThread;
    std::priority_queue<CurlTask> m_queues;
    boost::mutex m_mutex;
    int m_taskNums;
    bool m_running;
};