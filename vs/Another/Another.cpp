#if !defined(NDEBUG)
#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#define BOOST_THREAD_PROVIDES_FUTURE
#include <boost/call_traits.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/next_prior.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bimap.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time/custom_time_zone.hpp"
#include "boost/date_time/local_time/local_time_types.hpp"
#include "boost/date_time/local_time/tz_database.hpp"
#include "boost/date_time/local_time/posix_time_zone.hpp"
#include <boost/asio/steady_timer.hpp>
#include <boost/filesystem.hpp>
#include <queue>
#include <functional>
#include <utility>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")


using namespace boost::multi_index;
using namespace std;

/* A file record maintains some info on name and size as well
 * as a pointer to the directory it belongs (null meaning the root
 * directory.)
 */

struct file_entry
{
  file_entry(
    std::string name_,unsigned size_,bool is_dir_,const file_entry* dir_):
    name(name_),size(size_),is_dir(is_dir_),dir(dir_)
  {}

  std::string       name;
  unsigned          size;
  bool              is_dir;
  const file_entry* dir;

  friend std::ostream& operator<<(std::ostream& os,const file_entry& f)
  {
    os<<f.name<<"\t"<<f.size;
    if(f.is_dir)os<<"\t <dir>";
    return os;
  }
};

/* A file system is just a multi_index_container of entries with indices on
 * file and size. These indices are firstly ordered by directory, as commands
 * work on a current directory basis. Composite keys are just fine to model
 * this.
 * NB: The use of derivation here instead of simple typedef is explained in
 * Compiler specifics: type hiding.
 */

struct name_key:composite_key<
  file_entry,
  member<file_entry, const file_entry*, &file_entry::dir>,
  //BOOST_MULTI_INDEX_MEMBER(file_entry,const file_entry*,dir),
  BOOST_MULTI_INDEX_MEMBER(file_entry,std::string,name)
>{};

struct size_key:composite_key<
  file_entry,
  BOOST_MULTI_INDEX_MEMBER(file_entry,const file_entry* const,dir),
  BOOST_MULTI_INDEX_MEMBER(file_entry,unsigned,size)
>{};

/* see Compiler specifics: composite_key in compilers without partial
 * template specialization, for info on composite_key_result_less
 */

typedef multi_index_container<
  file_entry,
  indexed_by<
    /* primary index sorted by name (inside the same directory) */
    ordered_unique<
      name_key
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
      ,composite_key_result_less<name_key::result_type>
#endif
    >,
    /* secondary index sorted by size (inside the same directory) */
    ordered_non_unique<
      size_key
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
      ,composite_key_result_less<size_key::result_type>
#endif
    >
  >
> file_system;

/* typedef's of the two indices of file_system */
typedef nth_index<file_system,0>::type file_system_by_name;
typedef nth_index<file_system,1>::type file_system_by_size;

/* We build a rudimentary file system simulation out of some global
 * info and a map of commands provided to the user.
 */

static file_system fs;                 /* the one and only file system */
static file_system_by_name& fs_by_name=fs;         /* name index to fs */
static file_system_by_size& fs_by_size=get<1>(fs); /* size index to fs */
static const file_entry* current_dir=0;            /* root directory   */

/* command framework */

/* A command provides an execute memfun fed with the corresponding params
 * (first param is stripped off as it serves to identify the command
 * currently being used.)
 */

typedef boost::tokenizer<boost::char_separator<char> > command_tokenizer;

class command
{
public:
  virtual ~command(){}
  virtual void execute(
    command_tokenizer::iterator tok1,command_tokenizer::iterator tok2)=0;
};

/* available commands */

/* cd: syntax cd [.|..|<directory>] */

class command_cd:public command
{
public:
  virtual void execute(
    command_tokenizer::iterator tok1,command_tokenizer::iterator tok2)
  {
    if(tok1==tok2)return;
    std::string dir=*tok1++;

    if(dir==".")return;
    if(dir==".."){
      if(current_dir)current_dir=current_dir->dir;
      return;
    }

    file_system_by_name::iterator it=fs.find(
      boost::make_tuple(current_dir,dir));
    if(it==fs.end()){
      std::cout<<"non-existent directory"<<std::endl;
      return;
    }
    if(!it->is_dir){
      std::cout<<dir<<" is not a directory"<<std::endl;
      return;
    }
    current_dir=&*it;
  }
};
static command_cd cd;

/* ls: syntax ls [-s] */

class command_ls:public command
{
public:
  virtual void execute(
    command_tokenizer::iterator tok1,command_tokenizer::iterator tok2)
  {
    std::string option;
    if(tok1!=tok2)option=*tok1++;

    if(!option.empty()){
      if(option!="-s"){
        std::cout<<"incorrect parameter"<<std::endl;
        return;
      }

      /* list by size */

      file_system_by_size::iterator it0,it1;
      boost::tie(it0,it1)=fs_by_size.equal_range(
        boost::make_tuple(current_dir));
      std::copy(it0,it1,std::ostream_iterator<file_entry>(std::cout,"\n"));

      return;
    }

    /* list by name */

    file_system_by_name::iterator it0,it1;
    boost::tie(it0,it1)=fs.equal_range(boost::make_tuple(current_dir));
    std::copy(it0,it1,std::ostream_iterator<file_entry>(std::cout,"\n"));
  }
};
static command_ls ls;

/* mkdir: syntax mkdir <directory> */

class command_mkdir:public command
{
public:
  virtual void execute(
    command_tokenizer::iterator tok1,command_tokenizer::iterator tok2)
  {
    std::string dir;
    if(tok1!=tok2)dir=*tok1++;

    if(dir.empty()){
      std::cout<<"missing parameter"<<std::endl;
      return;
    }

    if(dir=="."||dir==".."){
      std::cout<<"incorrect parameter"<<std::endl;
      return;
    }

    if(!fs.insert(file_entry(dir,0,true,current_dir)).second){
      std::cout<<"directory already exists"<<std::endl;
      return;
    }
  }
};
static command_mkdir mkdir;

/* table of commands, a map from command names to class command pointers */

typedef std::map<std::string,command*> command_table;
static command_table cmt;

struct Node:boost::multi_index::identity<int>{};

class Foo
{
public:
    Foo()
    {
        std::cout << "default constructor" << std::endl;
    }
    Foo(const Foo& other)
    {
        std::cout << "copy constructor" << std::endl;
        m_foo = other.m_foo;
    }
    Foo& operator=(const Foo& other)
    {
       std::cout << "assign constructor" << std::endl;
        m_foo = other.m_foo;
        return *this;
    }
    int m_foo;
};

void accumulate(boost::promise<int> &p)
{
    int sum = 0;
    for (int i = 0; i < 5; ++i)
        sum += i;
    p.set_value(sum);
}


int accumulate()
{
    int sum = 0;
    for (int i = 0; i < 5; ++i)
        sum += i;
    return sum;
}


class BigObject {
public:
    BigObject() {
        cout << "constructor. " << endl;
    }
    ~BigObject() {
        cout << "destructor."<< endl;
    }
    BigObject(const BigObject&) {
        cout << "copy constructor." << endl;
    }
};

BigObject foo() {
    BigObject localObj;
    return localObj;
}

template<typename T>
struct MyAllocList{
    typedef std::list<T> type;
};


struct A
{
~A()
{
    std::cout << "~A()" << std::endl;
}
};

struct B
{
    ~B()
    {
        std::cout << "~B()" << std::endl;
    }
};

struct Task
{
    Task()
    {
        m_a.reset(new A());
    }
    virtual void Test()
    {
        std::cout << "base test" << std::endl;
    }
    virtual int LoadUdisk( const char* devicePath, bool bMounted)
    {
        std::cout << "base test" << std::endl;
        return 0;
    }
    std::shared_ptr<A> m_a;
};

struct Derived : public Task
{
public:
    Derived()
    {
        m_b.reset(new B);
    }
    virtual void Test()
    {
        Task::Test();
        std::cout << "derived test" << std::endl;
    }
    virtual int LoadUdisk( char* devicePath, bool bMounted)
    {
        std::cout << "base test" << std::endl;
        return 0;
    }
    std::shared_ptr<B> m_b;
};

class HasPtrMem
{
public:
    HasPtrMem() : m_ptr(new int(0))
    {
        std::cout << "construct:" << ++m_a << std::endl;
    }
    HasPtrMem(const HasPtrMem& other) : m_ptr(new int(*other.m_ptr))
    {
        std::cout << "copy construct:" << ++m_b << std::endl;
    }
    ~HasPtrMem()
    {
        delete m_ptr;
        m_ptr = NULL;
        std::cout << "destruct:" << ++m_c << std::endl;
    }

private:
    int * m_ptr;
    static int m_a;
    static int m_b;
    static int m_c;
};

int HasPtrMem::m_a = 0;
int HasPtrMem::m_b = 0;
int HasPtrMem::m_c = 0;

HasPtrMem GetObj()
{
    return HasPtrMem();
}

void func(const std::string& listenIp, const std::string& file)
{
    try
    {
        boost::asio::io_service io_service;
        using boost::asio::ip::udp;
        udp::socket socket(io_service, udp::endpoint(boost::asio::ip::address::from_string(listenIp), 9001));
        std::ofstream ofile(file.c_str(), std::ios::app);
        for (;;)
        {
            boost::array<char, 2048> recv_buf = {0};
            udp::endpoint remote_endpoint;
            boost::system::error_code error;
            size_t recv_len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);
            if (error && error != boost::asio::error::message_size)
            {
                std::cout << "size error" << std::endl;
                throw boost::system::system_error(error);
            }
            std::string ip2 = remote_endpoint.address().to_string();
            std::string ip = socket.local_endpoint().address().to_string();
            std::string data;
            std::string szMsg;
            szMsg.insert(szMsg.begin(), recv_buf.begin(), recv_buf.end());
            szMsg = szMsg.substr(0, recv_len);
            ofile << szMsg << std::endl;
            int len = szMsg.size();
            std::cout << "ip2:"  << ip2 << ", message:" << szMsg << std::endl;
        }
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cout << "my exception" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

void handler1(const boost::system::error_code &ec)   
{   
    std::cout <<  boost::this_thread::get_id() << " handler1." << std::endl;
    //sleep(3);
}   
void handler2(const boost::system::error_code &ec)   
{   
    std::cout <<  boost::this_thread::get_id() << " handler2." << std::endl; 
    Sleep(3);
}   
boost::asio::io_service io_service; 

void run()   
{   
    io_service.run();   
}   

static void CloseFile(FILE* fp)
{
    if (fp)
    {
        fflush(fp);
        fclose(fp);
    }
}

void ttt(void* file)
{

}

int main()
{
//     std::shared_ptr<boost::thread> pThread;
//     {
//         FILE* file = fopen("d:/aaaaaaa.txt", "wb");
//         std::shared_ptr<FILE> pFile(file, &CloseFile);
// 
//     }
//     boost::asio::io_service ioservice ;  
//     boost::asio::io_service my_io_service ;  
//     boost::asio::ip::tcp::resolver resolver(my_io_service);  
//     boost::asio::ip::tcp::resolver::query query("0.0.0.0", "9001");  
//     boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);  
//     boost::asio::ip::tcp::resolver::iterator end; // End marker.  
//     while (iter != end)  
//     {  
//         boost::asio::ip::tcp::endpoint endpoint = *iter++;  
//         std::cout << endpoint << std::endl;  
//     }  
   
//     boost::asio::deadline_timer timer1(io_service, boost::posix_time::seconds(2));   
//     timer1.async_wait(handler1);   
//     boost::asio::deadline_timer timer2(io_service, boost::posix_time::seconds(2));   
//     timer2.async_wait(handler2);   
//     boost::thread thread1(run);
//     boost::thread thread2(run);   
//     thread1.join();   
//     thread2.join();  
     std::shared_ptr<boost::thread> spThr(new boost::thread(boost::bind(&func, "0.0.0.0", "d:/net/10-3-70-3.txt")));
//     std::shared_ptr<boost::thread> spThr2(new boost::thread(boost::bind(&func, "192.168.1.100", "d:/net/192-168-1-100.txt")));
//     std::shared_ptr<boost::thread> spThr3(new boost::thread(boost::bind(&func, "127.0.0.1", "d:/net/127-0-1-1.txt")));
//     std::shared_ptr<boost::thread> spThr4(new boost::thread(boost::bind(&func, "0.0.0.0", "d:/net/0-0-0-0.txt")));
    //std::shared_ptr<boost::thread> spThr(new boost::thread(boost::bind(&func, "0.0.0.0", "d:/net/a.txt")));
    Sleep(1000000);
    //int res = inet_aton("239.83.51.79");
//     std::string res222 = inet_ntoa(22);
// 
// 
//     boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
// 
//     std::string snow = boost::posix_time::to_iso_string(now);
//     std::string snow2 = boost::posix_time::to_iso_extended_string(now);
// 
//     std::string ts("2017-06-14 02:16:08.786000");
//     boost::posix_time::ptime ptimedate = boost::posix_time::time_from_string(ts);
//     boost::posix_time::ptime a;
//     bool b = a.is_not_a_date_time();
//     bool c = ptimedate.is_not_a_date_time();
// 
//     boost::posix_time::ptime mgv_utc_time = boost::posix_time::ptime(boost::gregorian::date(2017, 1, 1));
//     std::string res = boost::posix_time::to_iso_string(ptimedate);
//     std::string res2 = boost::posix_time::to_iso_string(mgv_utc_time);
//     printf("Date : %s\n", __DATE__);
//     printf("Time : %s\n", __TIME__);
    
    
//     float deg = 270.0f / 360 * 2 * 3.1415926;
// 
//     float dx, dy, dr, db;
//     float deg2 = 330.0f / 360 * 2 * 3.1415926;
//     RotatePoint(0, 0, deg2, 100, 50, dx, dy);
//     RotatePoint(200, 100, deg2, 100, 50, dr, db);
//     float aaa = dx + (dr - dx) / 2.0f;
//     float bbb = dy + (db - dy) / 2.0f;
// 
// 
//     float nx  = -85, ny =120, nright = 423, nbottom = 406;
//     float ncx = 169, ncy = 272;
//     RotateHandler nrh(ncx, ncy, deg, nx, ny, nright, nbottom);
//     float nxx = FLT_MAX, nyy = FLT_MAX, nrr = FLT_MIN, nbb = FLT_MIN;
// 
//     float nLeft, nTop, nRight, nBottom;
// 
//     nrh.GetMinMaxRotated(nxx, nyy, nrr, nbb, nLeft, nTop, nRight, nBottom);
// 
//     float xx, yy, rr, bb;
//     //     RotatePoint(nLeft, nTop, deg, 169, 263, xx, yy);
//     //     RotatePoint(nRight, nBottom, deg, 169, 263, rr, bb);
//     
//     float m_cx = nLeft + (nRight - nLeft) / 2.0f;
//     float m_cy = nTop + (nBottom - nTop) / 2.0f;
//     RotatePoint(nLeft, nTop, deg, m_cx, m_cy, xx, yy);
//     RotatePoint(nRight, nBottom, deg, m_cx, m_cy, rr, bb);
// 
// 
//     float oxx = 0;
//     float oyy = 0;





//     boost::scoped_ptr<boost::asio::io_service> m_ioService;
//     boost::scoped_ptr<boost::asio::io_service::work> m_ioServiceWork;
//     boost::shared_ptr<boost::asio::steady_timer> m_timer;
// 
//     m_ioService.reset(new boost::asio::io_service);
//     m_ioServiceWork.reset(new boost::asio::io_service::work(*m_ioService));
//     m_timer.reset(new boost::asio::steady_timer(*m_ioService));
//     try
//     {
//         boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
//         boost::posix_time::ptime zeroTime(boost::gregorian::date(now.date() + boost::gregorian::days(1)), boost::posix_time::time_duration(0,0,0));
//         //boost::posix_time::ptime zeroTime(boost::gregorian::date(now.date().year(), now.date().month(), now.date().day() + 1),
//         //boost::posix_time::time_duration(0,0,0));
//         //zeroTime += boost::gregorian::days(1);
//         std::string aa = boost::posix_time::to_iso_extended_string(zeroTime);
//         boost::posix_time::time_duration d = zeroTime - now;
//         std::cout << d.total_seconds() << std::endl;
//         m_timer->expires_from_now(boost::chrono::seconds(d.total_seconds() + 1));
//         boost::posix_time::ptime lastWeek = boost::posix_time::second_clock::universal_time() - boost::gregorian::weeks(1);
//         m_timer->async_wait(boost::bind(&UpdateData, boost::asio::placeholders::error));
//         m_ioService->run();
//     }
//     catch (std::exception& e)
//     {
//         std::string a = e.what();
//         std::string other = a;
//     }
//     catch (...)
//     {
//         int a = 0;
//         a++;
//         int b = a;
//     }

//     const char* tags[] = {"eth0", "eth1", "wlan0"};
// 
//     for (int i = 0; i < sizeof(tags) / sizeof(const char *); ++i)
//     {
//         std::string ip = ip_tags[i];
//         std::cout << ip << std::endl;
//     }
// 
   
    system("pause");
}

//     // Do all your accepting and other stuff here.
//     std::string s = boost::lexical_cast<std::string>(a.local_endpoint());
//     int aaa = 0;
//     aaa ++;
    //std::string s = a.remote_endpoint().address().to_string();

//     boost::posix_time::ptime uni_time = boost::posix_time::second_clock::universal_time();
//     std::string szUniTime = boost::posix_time::to_iso_extended_string(uni_time);
//     boost::posix_time::ptime weekago = uni_time + boost::gregorian::weeks(1);
//     bool a = weekago  < uni_time;
//     std::string szUniTime2 = boost::posix_time::to_iso_extended_string(weekago);
//     std::string szUniTime3 = boost::posix_time::to_iso_extended_string(uni_time);


//     std::map<std::string, std::string> mmm;
//     mmm.insert(std::make_pair("hello", "wolrd"));
//     auto it = mmm.find("hello");
//     it->second = "he";
//     std::string res = mmm["hello"];
// 
//     using namespace boost::gregorian;
//     using namespace boost::posix_time;
//     using namespace boost::local_time;
//     tz_database tz_db;
//     tz_db.load_from_file("g:/mgv-ng-ssd/3rdlibs/boost/libs/date_time/data/date_time_zonespec.csv");
//     time_zone_ptr nyc_test = tz_db.time_zone_from_region("America/New_York");
//     
//     //time_zone_ptr tz(new posix_time_zone("CET+1"));
// 
//     boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
//     local_date_time dd(now, nyc_test);
//     std::string aa2 = dd.to_string();
//    // boost::posix_time::local_date_time dd = 
//     //std::string sznow = boost::posix_time::to_iso_extended_string(dd);
//     std::string aa = nyc_test->std_zone_abbrev();
//     std::string bb = aa;


//     int tzBias = -480;
//     boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
//     std::string sznow = boost::posix_time::to_iso_extended_string(now);
//     boost::posix_time::ptime utc = boost::posix_time::second_clock::universal_time() - boost::posix_time::time_duration(tzBias/60, 0, 0);
//     std::string szutc = boost::posix_time::to_iso_extended_string(now);
//     boost::posix_time::time_duration t = now - utc;
//     int diff = t.total_seconds() * 60 / 3600;
//     if (diff > 0)
//     {
//         tzBias -= diff;
//     }


//     std::string szutc = boost::posix_time::to_iso_extended_string(utc);
//     utc -= boost::posix_time::time_duration(tz/60, 0, 0);
//     std::string szutc2 = boost::posix_time::to_iso_extended_string(utc);



//     boost::posix_time::ptime old(boost::gregorian::date(now.date().year(), now.date().month(), now.date().day()),
//         boost::posix_time::time_duration(23,55,0));
//     std::string aa = boost::posix_time::to_iso_extended_string(old);
// 
//     boost::posix_time::ptime new2(boost::gregorian::date(now.date().year(), now.date().month(), now.date().day() + 1),
//         boost::posix_time::time_duration(0,0,0));
//     boost::posix_time::time_duration dddd =  new2 - old;
//     int ccc = dddd.total_seconds();
//     //boost::posix_time::ptime new2 = boost::posix_time::time_duration(1, 0, 0);
//     std::string aa2 = boost::posix_time::to_iso_extended_string(new2);
//     int aaa = old.time_of_day().hours();
// 
//     int dd = now.time_of_day().hours();
//     now += boost::posix_time::time_duration(1, 0, 0);
//     std::string  bb = boost::posix_time::to_iso_extended_string(now);
//     int d = now.time_of_day().hours();
//     int mm = now.time_of_day().minutes();
    



//     boost::promise<int> p;
//     boost::future<int> f = p.get_future();
//     boost::thread t(accumulate, std::ref(p));
//     int res = f.get();
//     std::cout << f.get() << '\n';
//     boost::packaged_task<int()> task(accumulate);
//     boost::future<int> f = task.get_future();
//     boost::thread t(std::move(task));
//     int a = f.get();
//     int b = a;

//     typedef boost::bimap<std::string, int> bimap;
//     bimap animals;
//     animals.left.insert(std::make_pair("cat", 4));
//     boost::bimap<std::string, int>::left_iterator it = animals.left.find("cat");
//     int bb = it->get_right();
//     std::string cc = it->get_left();
//     int aa = animals.left.count("cat");
//     std::cout << animals.left.count("cat") << '\n';
//     std::cout << animals.right.count(8) << '\n';

    //std::string  res= map["hello"];

  /* fill the file system with some data */

//   file_system::iterator it0,it1;
//   
//   fs.insert(file_entry("usr.cfg",240,false,0));
//   fs.insert(file_entry("memo.txt",2430,false,0));
//   it0=fs.insert(file_entry("dev",0,true,0)).first;
//     fs.insert(file_entry("tty0",128,false,&*it0));
//     fs.insert(file_entry("tty1",128,false,&*it0));
//   it0=fs.insert(file_entry("usr",0,true,0)).first;
//     it1=fs.insert(file_entry("bin",0,true,&*it0)).first;
//       fs.insert(file_entry("bjam",172032,false,&*it1));
//   it0=fs.insert(file_entry("home",0,true,0)).first;
//     it1=fs.insert(file_entry("andy",0,true,&*it0)).first;
//       fs.insert(file_entry("logo.jpg",5345,false,&*it1)).first;
//       fs.insert(file_entry("foo.cpp",890,false,&*it1)).first;
//       fs.insert(file_entry("foo.hpp",93,false,&*it1)).first;
//       fs.insert(file_entry("foo.html",750,false,&*it1)).first;
//       fs.insert(file_entry("a.obj",12302,false,&*it1)).first;
//       fs.insert(file_entry(".bash_history",8780,false,&*it1)).first;
//     it1=fs.insert(file_entry("rachel",0,true,&*it0)).first;
//       fs.insert(file_entry("test.py",650,false,&*it1)).first;
//       fs.insert(file_entry("todo.txt",241,false,&*it1)).first;
//       fs.insert(file_entry(".bash_history",9510,false,&*it1)).first;
// 
//   /* fill the command table */
// 
//   cmt["cd"]   =&cd;
//   cmt["ls"]   =&ls;
//   cmt["mkdir"]=&mkdir;
// 
//   /* main looop */
// 
//   for(;;){
//     /* print out the current directory and the prompt symbol */
// 
//     if(current_dir)std::cout<<current_dir->name;
//     std::cout<<">";
// 
//     /* get an input line from the user: if empty, exit the program */
// 
//     std::string com;
//     std::getline(std::cin,com);
//     command_tokenizer tok(com,boost::char_separator<char>(" \t\n"));
//     if(tok.begin()==tok.end())break; /* null command, exit */
// 
//     /* select the corresponding command and execute it */
// 
//     command_table::iterator it=cmt.find(*tok.begin());
//     if(it==cmt.end()){
//       std::cout<<"invalid command"<<std::endl;
//       continue;
//     }
// 
//     it->second->execute(boost::next(tok.begin()),tok.end());
//   }
