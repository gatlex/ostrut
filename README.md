ostrut *aka* ostream utilities
==============================


Frustrated because print-debugging with containers of the C++ standard library is sooo tedious?  If you're using C++11, then ostrut might come in handy. Simply
```c++
#include <ostrut/ostrut.hh>
```
and output your favourite instance of your favourite container type to your favourite `std::ostream`:
```c++
std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9};
std::cout << v << std::endl;
// Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9]
```
(Btw, you can find the examples of this document in [examples/basics.cc](examples/basics.cc).)

What you gain compared to a simple, manual two-line loop?  Let's consider a more complex structure
```c++
std::list<std::pair<std::vector<int>,std::map<std::string,std::vector<int>>>> cpx = {
  { {1,2,3}, { {"Hello", {42, 17}}, {"World", {21}} }},
  { {4,5,6}, { {"Olleh", {24, 71}}, {"Dlrow", {12}} }} };
```
So, nesting a `std::pair` of a `std::vector<int>` and a `std::map<std::string,std::vector<int>>` into a `std::list`.  Let alone its enigmatic definition, it should be much more difficult to display such a structure, right?  Well, no:
```c++
std::cout << cpx << std::endl;
// Outputs: [([1, 2, 3], {"Hello" : [42, 17], "World" : [21]}), ([4, 5, 6], {"Dlrow" : [12], "Olleh" : [24, 71]})]
```
You can see that pairs are displayed by round parentheses, while maps are displayed in Python-manner using braces and colons like `{A : B, C : D}`.  For more on that see [below on special syntax for common containers](#special-syntax).  Also notice that strings are wrapped in double quotes; similarly, characters would be wrapped in single quotes.


Usage
-----

The library is header only, so just include and be happy.  It heavily relies on C++11, so that's a prerequisite, but apart from that it should be platform independent (though I tested it only with g++ 4.9.1 so far).

Obtain the source by cloning the [github project](https://github.com/gatlex/ostrut).  For the impatient, the direct git-URL is [git@github.com:gatlex/ostrut.git](git@github.com:gatlex/ostrut.git).


Truncate Expansion of Long Containers
-------------------------------------

Truncate expansion of container with to many (e.g., an "infinity") of elements.  Suppose, for example, that you implemented a container type to iterate over all primes (in strictly monotonically increasing order).  Display the first 10 smallest primes by
```c++
std::cout << ostrut::exansion_limit(10) << primes() << std::endl;
// Outputs: [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, ...]
```

As usual, manipulators take effect until they are re-set:
```c++
std::cout << v << std::endl; // Let v be the std::vector<int> from above.
// Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, ...]
```

Switch back to unlimited expansion like so:
```c++
std::cout << ostrut::expansion_limit(ostrut::infinite) << v << std::endl;
// Outputs: [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9]
```

(Of course, after that don't
```c++
std::cout << primes() << std::endl;
```
unless your very (i.e., infinitely) patient.)

As already exemplified above, common standard containers are pimped to have a more intuitive syntax, e.g.:
```c++
std::map<int,int> m = { {1, 42}, {0x8, 15} };
std::cout << m << std::endl;
// {1 : 42, 8 : 15}
```


Special Syntax for Various Standard Containers<a name="special-syntax" />
----------------------------------------------

All iterateables can be output by `ostrut` with the syntax `[1, 2, 3, ...]` you already saw.  The only requirement is that the container implement the iterator interface (i.e., `std::begin(c)` and `std::end(c)` must be valid expressions for a const instance of the container,  yielding iterators).  Apart from this generic output, `ostrut` has some specializations for the most common standard library containers (for illustrations parametrized by integers):
- `std::map` is displayed like `{1 : 2, 2 : 3, ...}`.
- `std::set` is displayed like `{1, 2, 3, ...}`.
- `std::multiset` is displayed like `{|1, 1, 2, 3, 3, ...|}`.
- `std::multimap` is displayed like `{|1 : [2, 3, 4], 2 : [1], ...|}`.
For convenience, also pairs and tuples are displayed:
- `std::pair` is displayed like `(1, 2)`.
- `std::tuple` is displayed like `(1, 2, 3, 4, 5)`.
Also for convenience, pointers are dereferenced instead of displaying the raw memory address (supported are C-style pointers, `std::shared_ptr`, `std::unique_ptr`, `std::auto_ptr`, `std::weak_ptr`)


Matryoshka Limit
----------------

You already saw that `ostrut` can also handle nested containers.  For very deep structures, this might get confusing and a rought outline of the first few level might suffice.  This is where a nesting limit (aka matryoshka limit) enters the game (`cpx` being the complex nested data structure from the above example):
```c++
std::cout << ostrut::nesting_limit(2) << cpx << std::endl;
// Outputs: [([1, 2, 3], {"Hello" : [..], "World" : [..]}), ([4, 5, 6], {"Dlrow" : [..], "Olleh" : [..]})]
```
Of course, you can combine this with expansion limitation (making the output longer in this case):
```c++
std::cout << ostrut::nesting_limit(2) << ostrut::expansion_limit(2) << cpx << std::endl;
// Outputs: [([1, 2, ...], {"Hello" : [..], "World" : [..]}), ([4, 5, ...], {"Dlrow" : [..], "Olleh" : [..]})]
```

As before, switch back to unlimited nesting using `ostrut::nesting_limit(ostrut::infinite)`.

Oh, yeah: `ostrut::matryoshka_limit(n)` is a synomym for `ostrut::nesting_limit(n)` if you find that easier to memorize.


Pushing and Popping Settings
----------------------------

Suppose you have settings `std::cout << ostrut::nesting_limit(2) << ostrut::expansion_limit(2);` imposed that you want to change temporarily, e.g., to unbounded nesting and unbounded expansion.  This is what `ostrut::push_settings()` and `ostrut::pop_settings()` are for:
```c++
// cpx from above
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
```


Open Projects & Known Issues
----------------------------

- Specialization for `std::unordered_multimap`, see e.g. [here](http://en.cppreference.com/w/cpp/container).

- Specialization for Boost-variants (smart pointers, maps, tuples, pairs, ...).  (Use forward declarations as for standard library to minimize compile time.)

- Implement IO-manipulator for container-type-display like `ostrut::display_container_type(bool)` to indicate whether a high-level annotation of the container type should be included (e.g., `[<std::vector> 1, 2, 3, ...]`).

- Implement IO-manipulator for typeinfo-display like `ostrut::display_container_typeinfo(bool)` and `ostrut::display_element_typeinfo(bool)` to indicate whether a low-level annotation of the (container/element) type should be included (e.g., `[<std::vector<int> 1, 2, 3, ...]`).

- Implement IO-manipulator `ostrut::reset_settings()`.

- Incorporate g++ ABI-name-demangling for typeinfo. (NB, for VC++, typeinfo(...).name() is already descriptive.)

- Implement IO-manipulator to switch displaying memory addresses on/off, like `ostrut::display_pointer_addresses(bool)`, `ostrut::display_container_addresses(bool)`, and `ostrut::display_opaque_addresses(bool)` (only available if opaque type display is not disabled).  Also include a shortcut `ostrut::display_addresses(bool)` that is equivalent to setting all those addresses.

- Doxygen docu of the user interface.


- This document is a bit scanty (explain opaque type display, preprocessor parametrizations, tutorial on how to pimp own container types, synopsis of the interface).

- In particular this list is far from being complete :)

