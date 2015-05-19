// -*- coding: utf-8 -*-
#include <ostrut/ostrut.hh>

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <list>
#include <map>

// This is just for demonstration purposes and hopelessly inefficient
// (and incomplete for a container type at that).  Don't use it, or
// you deserve what you get!
struct primes {
  static bool is_prime(int n) {
    if(n < 2)
      return false;
    for(int i = 2; i < n; ++i)
      if((n % i) == 0)
	return false;
    return true;
  }
  
  struct const_iterator {
    int p_;
    const_iterator(int p)
      : p_(p)
    { }
    
    const_iterator &operator++() {
      if(p_ == 2)
	p_ = 3;
      else {
	do
	  p_ += 2;
	while(!is_prime(p_));
      }
      return *this;
    }
    
    const_iterator operator++(int) {
      const_iterator copy(*this);
      ++*this;
      return copy;
    }
    
    bool operator==(const const_iterator &other) const {
      return **this == *other;
    }
    
    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }
    
    int operator*() const {
      return p_;
    }
  };
    
  using iterator = const_iterator;
  
  const_iterator begin() const {
    return const_iterator(2);
  }
  
  const_iterator end() const {
    return const_iterator(0); // Just take no prime...
  }
};

int main(int, char **) {
  std::cout << "### Example 1:" << std::endl;
  std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9};
  std::cout << v << std::endl;
  // Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9]
  
  std::cout << "### Example 2:" << std::endl;
  std::list<std::pair<std::vector<int>,std::map<std::string,std::vector<int>>>> cpx = {
    { {1,2,3}, { {"Hello", {42, 17}}, {"World", {21}} }},
    { {4,5,6}, { {"Olleh", {24, 71}}, {"Dlrow", {12}} }} };
  std::cout << cpx << std::endl;
  // Outputs: [([1, 2, 3], {"Hello" : [42, 17], "World" : [21]}), ([4, 5, 6], {"Dlrow" : [12], "Olleh" : [24, 71]})]
  
  std::cout << "### Example 3:" << std::endl;
  std::cout << ostrut::expansion_limit(10) << primes() << std::endl;
  // Outputs (assuming primes() and my arithmetics still work): [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, ...]
  
  std::cout << "### Example 4:" << std::endl;
  std::cout << v << std::endl; // Let v be the std::vector<int> from above.
  // Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, ...]
  
  std::cout << "### Example 5:" << std::endl;
  std::cout << ostrut::expansion_limit(ostrut::infinite) << v << std::endl;
  // Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9]
  
  std::cout << "### Example 6:" << std::endl;
  std::map<int,int> m = { {1, 42}, {0x8, 15} };
  std::cout << m << std::endl;
  // {1 : 42, 8 : 15}
  
  std::cout << "### Example 7:" << std::endl;
  std::cout << ostrut::nesting_limit(2) << cpx << std::endl;
  // Outputs: [([1, 2, 3], {"Hello" : [..], "World" : [..]}), ([4, 5, 6], {"Dlrow" : [..], "Olleh" : [..]})]
  
  std::cout << "### Example 8:" << std::endl;
  std::cout << ostrut::nesting_limit(2) << ostrut::expansion_limit(2) << cpx << std::endl;
  // Outputs: [([1, 2, ...], {"Hello" : [..], "World" : [..]}), ([4, 5, ...], {"Dlrow" : [..], "Olleh" : [..]})]
  
  std::cout << "### Example 9:" << std::endl;
  std::cout << cpx << std::endl
	    << ostrut::push_settings()
	    << ostrut::nesting_limit(ostrut::infinite) << ostrut::expansion_limit(ostrut::infinite)
	    << cpx << std::endl
	    << ostrut::pop_settings()
	    << cpx << std::endl;
  // Outputs:
  //   [([1, 2, ...], {"Hello" : [..], "World" : [..]}), ([4, 5, ...], {"Dlrow" : [..], "Olleh" : [..]})]
  //   [([1, 2, 3], {"Hello" : [42, 17], "World" : [21]}), ([4, 5, 6], {"Dlrow" : [12], "Olleh" : [24, 71]})]
  //   [([1, 2, ...], {"Hello" : [..], "World" : [..]}), ([4, 5, ...], {"Dlrow" : [..], "Olleh" : [..]})]
  
  return 0;
}
