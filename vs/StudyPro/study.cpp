#include <iostream>
//ref http://www.codeproject.com/Tips/1023429/The-Virtual-Inheritance-and-Funny-Tricks

class Base
{
public:
	Base(int n) : value(n)
	{ 
		std::cout << "Base(" << n << ")"<< std::endl; // Prints the passed value: Base(N)
	}
	Base() : value(0)
	{ 
		std::cout << "Base()"<< std::endl; // No passed value: Base()
	}
	~Base() { std::cout << "~Base()"<< std::endl; }

	int value;
};

class One : public  virtual Base
{
public:
	One() : Base(1) 
	{ 
		std::cout << "One()"<< std::endl; 
	}
	~One() { std::cout << "~One()"<< std::endl; }
};

class Two : public virtual Base
{
public:
	Two() : Base(2)
	{ 
		std::cout << "Two()"<< std::endl; 
	}
	~Two() { std::cout << "~Two()"<< std::endl; }
};

class Leaf : public One, public Two
{
public:
	Leaf() { std::cout << "Leaf()"<< std::endl; }
	~Leaf() { std::cout << "~Leaf()"<< std::endl; }
};

//final class
class Seal
{
	friend class Final;
	Seal() {}
};

class Final : public virtual Seal
{
public:
	Final() {}
};

// call virtual function in contructors.

class base2;

class caller_helper
{
public:
	caller_helper() : m_p(nullptr) {}
	void init(base2* p) const { m_p = p; }
	~caller_helper();
private:
	mutable base2* m_p;
};

class base2
{
public:
	base2(const caller_helper& caller) 
	{	
		caller.init(this); // store the pointer
	}
	virtual void to_override(){} // empty virtual function
};

class derived : public virtual base2
{
public:
	derived(const caller_helper& caller = caller_helper()) : base2(caller) {}
	virtual void to_override()
	{
		std::cout << "derived"<< std::endl;
	}
};

class derived_derived : public derived
{
public:
	derived_derived(const caller_helper& caller = caller_helper()) : base2(caller) {}
	virtual void to_override()
	{
		std::cout << "derived_derived"<< std::endl;
	}
};

caller_helper::~caller_helper()
{
	if(m_p) // important not to call to not initialized
		m_p->to_override();
}
int main()
{
    Leaf lf;
	lf.value = 3;
	/*
	Base() // NOTE: Neither "Base(1)" nor "Base(2)"
	One()
	Two()
	Leaf()
	*/
	derived d;
	derived_derived dd;
	/*
	derived
	derived_derived
	*/
	system("pause");
    return 0;
}