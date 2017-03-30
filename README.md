std::move无条件地把它的参数转换成一个右值，而std::forward只在特定条件满足的情况下执行这个转换
c++ 14:
template<typename T>
decltype(auto) move(T&& param)
{
    using ReturnType = remove_reference_t<T>&&;
    return static_cast<ReturnType>(param);
}
并且C++的众多”可爱“的规则中的一个就是，在依赖类型前面必须加typename
别名模板(alias template)
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;
std::remove_const<T>::type          //C++11: const T -> T
std::remove_const_t<T>              //C++14 等价的操作
std::remove_reference<T>::type      // C++11: T&/T&& → T
std::remove_reference_t<T>          // C++14 等价的操作
std::add_lvalue_reference<T>::type  // C++11: T → T&
std::add_lvalue_reference_t<T>      // C++14 等价的操作

template <class T>
using remove_const_t = typename remove_const<T>::type;
template <class T>
using remove_reference_t = typename remove_reference<T>::type;
template <class T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;


1.phase阶段/feiz/ mimic模仿 mnemonics/nəˈmäniks/助记符 conventional常规

2.
Copy constructor	
X x(y);
X x = y;
Assignment	x = y;

3.
template<typename T0,...,typename Tn>
struct indexed_by;
//助记符
tag
identity //对于一般的变量
member //对于成员变量
member_offset
const_mem_fun
mem_fun
const_mem_fun_explicit // for vc6.0
mem_fun_explicit
global_fun //对于一般的函数
composite_key
composite_key_result
composite_key_equal_to
composite_key_compare
composite_key_result_less
composite_key_result_greater
composite_key_hash
composite_key_result_hash

A multi_index_container's node is basically a "multinode"
with as many headers as indices as well as the payload.
For instance, a multi_index_container with two so-called
ordered indices uses an internal node that looks like
struct node
{
  // header index #0
  color      c0;
  pointer    parent0,left0,right0;
  // header index #1
  color      c1;
  pointer    parent1,left1,right2;
  // payload
  value_type value;
};




boost::tie
composite_key

# study
chorium:
http://www.cnblogs.com/qq499194341/articles/2917432.html

chromium md:
https://github.com/fanfeilong/cefutil/tree/master/doc