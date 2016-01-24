#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/windows_shared_memory.hpp> 
#include <boost/interprocess/managed_shared_memory.hpp> 
#include <string>
#include <iostream>
struct AA {
	AA(int a, std::string b):m_a(a), m_b(b)
	{

	}
	int m_a;
	std::string m_b;
};
int main(int argc, char* argv[ ]) 
{
	//Windows 提供了一种特别的共享内存，它可以在最后一个使用它的应用程序终止后自动删除
	using namespace boost::interprocess; 
	try { 
// 		boost::interprocess::windows_shared_memory shdmem(boost::interprocess::open_or_create, "Highscore", boost::interprocess::read_write, 1024); 
// 		boost::interprocess::mapped_region region(shdmem, boost::interprocess::read_write); 
// 		int *i1 = static_cast<int*>(region.get_address()); 
// 		*i1 = 99; 
// 		boost::interprocess::mapped_region region2(shdmem, boost::interprocess::read_only); 
// 		int *i2 = static_cast<int*>(region2.get_address()); 
// 		int res = *i2;
// 		std::cout << *i2 << std::endl; 
	
		boost::interprocess::shared_memory_object::remove("Highscore"); 
		boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::open_or_create, "Highscore", 1024);
	

		AA *i = managed_shm.construct<AA>("AA")(1, "nihao"); 
		int aa = i->m_a;
		std::string bb = i->m_b;
		int c;
// 		std::cout << *i << std::endl; 
// 		std::pair<int*, std::size_t> p = managed_shm.find<int>("Integer"); 
// 		if (p.first) 
// 			std::cout << *p.first << std::endl; 
	} catch (interprocess_exception& e) { 
		// .. .  clean up 
	}
	system("pause");
}