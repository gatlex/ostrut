// -*- coding: utf-8 -*-
#pragma once
#ifndef OSTREAM_UTILITIES_HPP_
#define OSTREAM_UTILITIES_HPP_


// # Precompiler parametrization

#ifndef OSTRUT_DEFAULT_EXPANSION_LIMIT
#define OSTRUT_DEFAULT_EXPANSION_LIMIT ::ostrut::infinite
#endif // ifndef OSTRUT_DEFAULT_EXPANSION_LIMIT

#ifndef OSTRUT_DEFAULT_EXPANSION_CANCELED_STRING
#define OSTRUT_DEFAULT_EXPANSION_CANCELED_STRING std::string("...")
#endif // ifndef OSTRUT_DEFAULT_EXPANSION_CANCELED_STRING

#ifndef OSTRUT_DEFAULT_NESTING_LIMIT
#define OSTRUT_DEFAULT_NESTING_LIMIT ::ostrut::infinite
#endif // ifndef OSTRUT_DEFAULT_NESTING_LIMIT

#ifndef OSTRUT_DEFAULT_NESTING_CANCELED_STRING
#define OSTRUT_DEFAULT_NESTING_CANCELED_STRING std::string("..")
#endif // ifndef OSTRUT_DEFAULT_NESTING_CANCELED_STRING

#ifndef OSTRUT_DEFAULT_NULLPTR_STRING
#define OSTRUT_DEFAULT_NULLPTR_STRING std::string("null")
#endif // ifndef OSTRUT_DEFAULT_NULLPTR_STRING

#ifndef OSTRUT_NO_OPAQUE_OUTPUT
#include <typeinfo> // typeid
#endif // ifndef OSTRUT_NO_OPAQUE_OUTPUT

#ifndef OSTRUT_NO_SETTINGS_STACK
#include <vector>
#endif // ifndef OSTRUT_NO_SETTINGS_STACK

#ifndef OSTRUT_NO_RECURSION_ABORTION

#ifndef OSTRUT_DEFAULT_RECURSION_ABORTION_STRING
#define OSTRUT_DEFAULT_RECURSION_ABORTION_STRING std::string("@")
#endif // ifndef OSTRUT_DEFAULT_RECURSION_ABORTION_STRING

#include <vector>
#endif // ifndef OSTRUT_NO_RECURSION_ABORTION


// # Forward declarations

namespace std {
  // The following forward declarations are to minimize compilation
  // time.  Incomplete declarations suffice here because outputting
  // them, the user will have to have included the respective headers
  // theirselves, anyway.
  // 
  // If you're compiler is complaining about to few template
  // parameters in one of the following lines, then you probably
  // forgot to include the file that defines the respective container.
  
  // #include <map>
  template<typename key, typename t, typename compare, typename allocator> class map;
  template<typename key, typename t, typename compare, typename allocator> class multimap; 

  // #include <unordered_map>
  template<typename key, typename t, typename hash, typename key_equal, typename allocator> class unordered_map;
  //template<typename key, typename t, typename hash, typename key_equal, typename allocator> class unordered_multimap;
  
  // #include <set>
  template<typename key, typename compare, typename allocator> class set;
  template<typename key, typename compare, typename allocator> class multiset;
  
  // #include <unordered_set>
  template<typename key, typename hash, typename key_equal, typename allocator> class unordered_set;
  template<typename key, typename hash, typename key_equal, typename allocator> class unordered_multiset;
  
  // #include <memory>
  template<typename t> class shared_ptr;
  template<typename t> class weak_ptr;
  template<typename t> class auto_ptr;
  template<typename t, typename deleter> class unique_ptr;

  // #include <string>
  //template<typename char_t, typename traits, typename allocator> class basic_string;
  
  // #include <utility>
  //template<typename t1, typename t2> struct pair;
  
  // #include <tuple>
  //template<typename... types> class tuple;
  
  // #include <iostream>
  template<typename char_t, typename traits> class basic_ostream;
  
}


// # Includes, forward declarations won't do here (or are to laborious)
#include <type_traits> // std::is_pointer and the like
#include <limits> // std::numeric_limits<size_t>::max()
#include <locale> // std::locale, facets
#include <string> // std::basic_string specialization std::string
#include <tuple>  // std::get

#ifdef NDEBUG
#define assert(x) ((void)0)
#else // ifndef NDEBUG
#include <cassert>
#endif // ifndef-else NDEBUG

// # Library details
namespace ostrut { // Handling representation introspection into
		   // container content; helpers for dispatching to
		   // different container types.

  namespace detail {
    // Using SFINAE, this will only be instantiated if all parameters are valid.
    template<typename result_t, typename... types>
    struct enable_if_all : std::true_type { using type = result_t; };
    
    // Specialize for smart pointers, exporting t as type.
    // Don't include weak_ptr. To dereference weak_ptr, it has to be
    // locked first, so it's not a pointer type in that sense.
    template<typename t> struct is_smart_pointer : std::false_type {};
    template<typename t> struct is_smart_pointer<std::shared_ptr<t> > : std::true_type {};
    template<typename t, typename deleter> struct is_smart_pointer<std::unique_ptr<t, deleter> > : std::true_type {};
    template<typename t> struct is_smart_pointer<std::auto_ptr<t>   > : std::true_type {};
    
    // Specialize for weak pointers, exporting t as type.
    template<typename t> struct is_weak_pointer : std::false_type {};
    template<typename t> struct is_weak_pointer<std::weak_ptr<t> > : std::true_type {};
    
    // Specialize for hash maps.
    template<typename t> struct is_map : std::false_type {};
    template<typename key, typename val, typename hash, typename key_equal, typename allocator>
    struct is_map<std::unordered_map<key,val,hash,key_equal,allocator> > : std::true_type {};
    template<typename key, typename val, typename compare, typename allocator>
    struct is_map<std::map<key,val,compare,allocator> > : std::true_type {};
    
    // Specialize for sets.
    template<typename t> struct is_set : std::false_type {};
    template<typename key, typename compare, typename allocator>
    struct is_set<std::set<key,compare,allocator> > : std::true_type {};

    // Specialize for multisets.
    template<typename t>
    struct is_multiset : std::false_type {};
    template<typename key, typename compare, typename allocator>
    struct is_multiset<std::multiset<key,compare,allocator> > : std::true_type {};
    template<typename key, typename hash, typename key_equal, typename allocator>
    struct is_multiset<std::unordered_multiset<key,hash,key_equal,allocator> > : std::true_type {};
    
    // Specialize for multimaps.
    template<typename t>
    struct is_multimap : std::false_type {};
    template<typename key, typename value_t, typename compare, typename allocator>
    struct is_multimap<std::multimap<key,value_t,compare,allocator> > : std::true_type {};
    
    // Captures all other containers that are not special in one of
    // the above senses.
    template<typename type_>
    struct is_generic_container {
      using type = type_;
      static const bool value = !(is_set<type>::value or 
				  is_multiset<type>::value or 
				  is_map<type>::value or 
				  is_multimap<type>::value or 
				  false);
    };
    
    // Helper to detect whether type-instances are streamable to
    // ostream_t-instances.  Used for opaque objects.
    template<typename ostream_t, typename type, typename=void>
    struct is_ostreamable : std::false_type {};
    template<typename ostream_t, typename type>
    struct is_ostreamable<ostream_t, type, decltype(std::declval<ostream_t &>() << std::declval<type>(), void())> : std::true_type {};
    
    // Helper to 'introspect' special types before outputting in
    // containers, pairs and other composites.  Used, e.g., to put
    // strings and chars in (double/single) quotes.  Also used to
    // display type information about opaque objects (if not
    // disabled).
    template<typename type_>
    struct introspect {
      template<typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const type_ &c)
	-> std::basic_ostream<char_t, _traits> &
      {
	return repr<type_, char_t, _traits>(out, c);
      }

    private:
      template<typename type, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const type &c)
	-> typename std::enable_if<is_ostreamable<std::basic_ostream<char_t, _traits>, type>::value,
	                           std::basic_ostream<char_t, _traits> &>::type
      {
	return out << c;
      }
#ifndef OSTRUT_NO_OPAQUE_OUTPUT
      template<typename type, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const type &c)
	-> typename std::enable_if<!is_ostreamable<std::basic_ostream<char_t, _traits>, type>::value,
	                           std::basic_ostream<char_t, _traits> &>::type
      {
        return out << "object @" << &c << " of type " << typeid(decltype(c)).name();
      }
#else // ifndef OSTRUT_NO_OPAQUE_OUTPUT
      template<typename type, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const type &c)
	-> typename std::enable_if<!is_ostreamable<std::basic_ostream<char_t, _traits>, type>::value,
	                           std::basic_ostream<char_t, _traits> &>::type
      {
	static_assert(is_ostreamable<std::basic_ostream<char_t, _traits>, type>::value, 
		      "Tried to display opaque type with OSTRUT_NO_OPAQUE_OUTPUT defined; "
		      "either overload operator<< to enable opaque type information display.");
      }
#endif  // ifndef-else OSTRUT_NO_OPAQUE_OUTPUT
    };
    
    // Wrap string in double quotes.
    template<typename str_char_t, typename str_traits, typename str_allocator>
    struct introspect<std::basic_string<str_char_t,str_traits,str_allocator> > {
      template<typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const std::basic_string<str_char_t,str_traits,str_allocator> &s)
	-> typename enable_if_all<std::basic_ostream<char_t, _traits> &,
				  decltype(out << s)>::type
      {
	out << '\"';
	for(size_t i = 0; i < s.length(); ++i) {
	  if(s[i] != '\"')
	    out << s[i];
	  else
	    out << "\\\"";
	}
	return out << '\"';
      }
    };
    // Wrap string in double quotes.
    template<>
    struct introspect<char *> {
      template<typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const char *s)
	-> typename enable_if_all<std::basic_ostream<char_t, _traits> &,
				  decltype(out << s)>::type
      {
	return introspect<std::string>::repr(out, std::string(s));
      }
    };
    // Wrap char in single quotes.
    template<>
    struct introspect<char> {
      template<typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, char c)
	-> typename enable_if_all<std::basic_ostream<char_t, _traits> &,
				  decltype(out << c)>::type
      {
	return out << "\'" << c << "\'";
      }
    };
    // Expand tuple as a list (without parentheses).
    template<size_t i>
    struct introspect_tuple {
      template<typename tuple_t, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const tuple_t &t)
	-> typename enable_if_all<std::basic_ostream<char_t, _traits> &,
				  decltype(introspect<typename std::remove_const<typename std::remove_reference<
					   decltype(std::get<std::tuple_size<tuple_t>::value-i>(t))>::type>::type>::
					   repr(out, std::get<std::tuple_size<tuple_t>::value-i>(t))),
				  decltype(introspect_tuple<i-1>::repr(out, t))
				  >::type
      {
	using value_t = typename std::remove_const<typename std::remove_reference<
	  decltype(std::get<std::tuple_size<tuple_t>::value-i>(t))>::type>::type;
	return introspect_tuple<i-1>::repr(introspect<value_t>::repr(out, std::get<std::tuple_size<tuple_t>::value-i>(t)) << ", ", t);
      }
    };
    template<>
    struct introspect_tuple<1> {
      template<typename tuple_t, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const tuple_t &t)
    	-> typename enable_if_all<std::basic_ostream<char_t, _traits> &,
				  decltype(out << std::get<std::tuple_size<tuple_t>::value-1>(t))
	                          >::type
      {
    	return out << std::get<std::tuple_size<tuple_t>::value-1>(t);
      }
    };
    template<>
    struct introspect_tuple<0> {
      template<typename tuple_t, typename char_t, typename _traits>
      static auto repr(std::basic_ostream<char_t, _traits> &out, const tuple_t &t)
	-> decltype(out)
      {
	return out;
      }
    };

    // Stolen from here: http://stackoverflow.com/a/12919396/2473926
    // (Unused, but nice and may come in handy.)
    template<template <typename...> class template_t, typename t>
    struct is_specialization_of : std::false_type {
      static const bool value = false;
    };
    
    template<template <typename...> class template_t, typename... args >
    struct is_specialization_of<template_t, template_t<args...>> : std::true_type {
      static const bool value = true;
    };
    
  } // namespace detail
} // namespace ostrut


// # Interlude: Some internal stuff for the internal state of the lib
namespace ostrut {
  namespace manip {
    namespace detail {
      struct infinite_t { };
      struct state_facet;

      /// Returns the facet that contains the internal state.  Imbues one if not yet existing.
      template<typename char_t, typename _traits>
      state_facet &getimbue_state_facet(std::basic_ostream<char_t, _traits> &out);
      
      /// Returns the facet that contains the internal state.  Imbues one if not yet existing.
      template<typename char_t, typename _traits>
      const state_facet &getimbue_state_facet(const std::basic_ostream<char_t, _traits> &out);
      
    } // namespace detail  
  } // namespace manip
  
  /// Use e.g. for std::cout << ostrut::expansion_limit(ostrut::infinite).
  static const manip::detail::infinite_t infinite;
  
} // namespace ostrut


// # The main interface, injecting operator<< into std for various containers.
namespace std { // Inject operator<< into std so that the compiler finds it.

  // Make any iteratable container ostream-able (whose contained type
  // is).  Deactivate if
  // ostrut::detail::is_generic_container<container_t>::value is false
  // in order to avoid ambiguous template specializations.
  template<typename container_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const container_t &c)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
						typename std::enable_if<::ostrut::detail::is_generic_container<container_t>::value>::type,
					        decltype(std::begin(c) != std::end(c)), 
					        decltype(::ostrut::detail::introspect<decltype(*std::begin(c))>::repr(out, *std::begin(c)))>::type
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
#ifndef OSTRUT_NO_RECURSION_ABORTION
    if(ostrut_state.callstack_contains(&c))
      return out << ostrut_state.recursion_abortion_string();
    else
      ostrut_state.callstack_push_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    
    out << "[";
    if(ostrut_state.current_nesting_depth >= ostrut_state.matryoshka_limit())
      out << ostrut_state.nesting_canceled_string();
    else {
      ++ostrut_state.current_nesting_depth;
      
      auto i = std::begin(c);
      using value_t = typename std::remove_const<typename std::remove_reference<decltype(*i)>::type>::type;
      if(i != std::end(c)) {
	auto expansion_counter = ostrut_state.expansion_limit();
	if(expansion_counter <= 0)
	  out << ostrut_state.expansion_canceled_string();
	else {
	  --expansion_counter;
	  ::ostrut::detail::introspect<value_t>::repr(out, *i++);
	  while(i != std::end(c)) {
	    if(expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    --expansion_counter;
	    ::ostrut::detail::introspect<value_t>::repr(out << ", ", *i++);
	  }
	}
      }
      
      --ostrut_state.current_nesting_depth;
    }
#ifndef OSTRUT_NO_RECURSION_ABORTION
    ostrut_state.callstack_pop_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    return out << "]";
  }

  // Pimp map types.
  template<typename container_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const container_t &c)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
						typename std::enable_if<::ostrut::detail::is_map<container_t>::value>::type,
					        decltype(std::begin(c) != std::end(c)),
					        decltype(out << (*std::begin(c)).first),
					        decltype(out << (*std::begin(c)).second)>::type
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    
#ifndef OSTRUT_NO_RECURSION_ABORTION
    if(ostrut_state.callstack_contains(&c))
      return out << ostrut_state.recursion_abortion_string();
    else
      ostrut_state.callstack_push_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    
    out << "{";
    if(ostrut_state.current_nesting_depth >= ostrut_state.matryoshka_limit())
      out << ostrut_state.nesting_canceled_string();
    else {
      ++ostrut_state.current_nesting_depth;
      
      auto i = std::begin(c);
      using key_t = typename std::remove_const<typename std::remove_reference<decltype((*i).first)>::type>::type;
      using value_t = typename std::remove_const<typename std::remove_reference<decltype((*i).second)>::type>::type;
      if(i != std::end(c)) {
	const auto e = *i++;
	auto expansion_counter = ostrut_state.expansion_limit();
	if(expansion_counter <= 0)
	  out << ostrut_state.expansion_canceled_string();
	else {
	  --expansion_counter;
	  ::ostrut::detail::introspect<value_t>::repr(::ostrut::detail::introspect<key_t>::repr(out, e.first) << " : ", e.second);
	  while(i != std::end(c)) {
	    if(expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    --expansion_counter;
	    const auto e = *i++;
	    ::ostrut::detail::introspect<value_t>::repr(::ostrut::detail::introspect<key_t>::repr(out << ", ", e.first) << " : ", e.second);
	  }
	}
      }
      
      --ostrut_state.current_nesting_depth;
    }
#ifndef OSTRUT_NO_RECURSION_ABORTION
    ostrut_state.callstack_pop_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    return out << "}";
  }

  // Multiset types.
  template<typename container_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const container_t &c)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
						typename std::enable_if<::ostrut::detail::is_multiset<container_t>::value>::type,
					        decltype(std::begin(c) != std::end(c)),
					        decltype(out << *std::begin(c))>::type
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    
#ifndef OSTRUT_NO_RECURSION_ABORTION
    if(ostrut_state.callstack_contains(&c))
      return out << ostrut_state.recursion_abortion_string();
    else
      ostrut_state.callstack_push_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    
    out << "{|";
    if(ostrut_state.current_nesting_depth >= ostrut_state.matryoshka_limit())
      out << ostrut_state.nesting_canceled_string();
    else {
      ++ostrut_state.current_nesting_depth;
      
      auto i = std::begin(c);
      using key_t = typename std::remove_const<typename std::remove_reference<decltype(*i)>::type>::type;
      if(i != std::end(c)) {
	const auto e = *i++;
	auto expansion_counter = ostrut_state.expansion_limit();
	if(expansion_counter <= 0)
	  out << ostrut_state.expansion_canceled_string();
	else {
	  --expansion_counter;
	  ::ostrut::detail::introspect<key_t>::repr(out, e);
	  while(i != std::end(c)) {
	    if(expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    --expansion_counter;
	    const auto e = *i++;
	    ::ostrut::detail::introspect<key_t>::repr(out << ", ", e);
	  }
	}
      }
      
      --ostrut_state.current_nesting_depth;
    }
#ifndef OSTRUT_NO_RECURSION_ABORTION
    ostrut_state.callstack_pop_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    return out << "|}";
  }
  
  // Multimap types.
  template<typename container_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const container_t &c)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
						typename std::enable_if<::ostrut::detail::is_multimap<container_t>::value>::type,
					        decltype(std::begin(c) != std::end(c)),
					        decltype(out << std::begin(c)->first),
					        decltype(out << std::begin(c)->second)>::type
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    
#ifndef OSTRUT_NO_RECURSION_ABORTION
    if(ostrut_state.callstack_contains(&c))
      return out << ostrut_state.recursion_abortion_string();
    else
      ostrut_state.callstack_push_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    
    out << "{|";
    if(ostrut_state.current_nesting_depth >= ostrut_state.matryoshka_limit())
      out << ostrut_state.nesting_canceled_string();
    else {
      ++ostrut_state.current_nesting_depth;
      
      auto j = std::begin(c);
      using key_t = typename std::remove_const<typename std::remove_reference<decltype(j->first)>::type>::type;
      using val_t = typename std::remove_const<typename std::remove_reference<decltype(j->second)>::type>::type;
      if(j != std::end(c)) {
	auto expansion_counter = ostrut_state.expansion_limit();
	if(expansion_counter <= 0)
	  out << ostrut_state.expansion_canceled_string();
	else {
	  --expansion_counter;
	  ::ostrut::detail::introspect<key_t>::repr(out, j->first) << " : [";
	  auto inner_expansion_counter = ostrut_state.expansion_limit();
	  if(inner_expansion_counter <= 0)
	    out << ostrut_state.expansion_canceled_string();
	  else {
	    ::ostrut::detail::introspect<val_t>::repr(out, j->second);
	    --inner_expansion_counter;
	  }
	  auto i = j;
	  for(++j; j != std::end(c) and j->first == i->first; ++j) { // Hack(?): Should I use a comparator? What does multimap itself use?
	    if(inner_expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    else {
	      ::ostrut::detail::introspect<val_t>::repr(out << ", ", j->second);
	      --inner_expansion_counter;
	    }
	  }
	  for(; j != std::end(c) and j->first == i->first; ++j) {}
	  out << "]";
	
	  i = j;
	  while(j != std::end(c)) {
	    if(expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    --expansion_counter;
	    ::ostrut::detail::introspect<key_t>::repr(out << ", ", j->first) << " : [";
	    auto inner_expansion_counter = ostrut_state.expansion_limit();
	    if(inner_expansion_counter <= 0)
	      out << ostrut_state.expansion_canceled_string();
	    else {
	      ::ostrut::detail::introspect<val_t>::repr(out, j->second);
	      --inner_expansion_counter;
	    }
	    for(++j; j != std::end(c) and j->first == i->first; ++j) { // Hack(?): Should I use a comparator? What does multimap itself use?
	      if(inner_expansion_counter <= 0) {
		out << ", " << ostrut_state.expansion_canceled_string();
		break;
	      }
	      else {
		::ostrut::detail::introspect<val_t>::repr(out << ", ", j->second);
		--inner_expansion_counter;
	      }
	    }
	    for(; j != std::end(c) and j->first == i->first; ++j) {}
	    out << "]";
	  
	    i = j;
	  }
	}
      }
      
      --ostrut_state.current_nesting_depth;
    }
#ifndef OSTRUT_NO_RECURSION_ABORTION
    ostrut_state.callstack_pop_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    return out << "|}";
  }
  
  // Pimp set types.
  template<typename container_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const container_t &c)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
						typename std::enable_if<::ostrut::detail::is_set<container_t>::value>::type,
					        decltype(std::begin(c) != std::end(c)),
					        decltype(out << *std::begin(c))>::type
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    
#ifndef OSTRUT_NO_RECURSION_ABORTION
    if(ostrut_state.callstack_contains(&c))
      return out << ostrut_state.recursion_abortion_string();
    else
      ostrut_state.callstack_push_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    
    out << "{";
    if(ostrut_state.current_nesting_depth >= ostrut_state.matryoshka_limit())
      out << ostrut_state.nesting_canceled_string();
    else {
      ++ostrut_state.current_nesting_depth;
      
      auto i = std::begin(c);
      using key_t = typename std::remove_const<typename std::remove_reference<decltype(*i)>::type>::type;
      if(i != std::end(c)) {
	auto expansion_counter = ostrut_state.expansion_limit();
	if(expansion_counter <= 0)
	  out << ostrut_state.expansion_canceled_string();
	else {
	  --expansion_counter;
	  const auto e = *i++;
	  ::ostrut::detail::introspect<key_t>::repr(out, e);
	  while(i != std::end(c)) {
	    if(expansion_counter <= 0) {
	      out << ", " << ostrut_state.expansion_canceled_string();
	      break;
	    }
	    --expansion_counter;
	    const auto e = *i++;
	    ::ostrut::detail::introspect<key_t>::repr(out << ", ", e);
	  }
	}
      }
      
      --ostrut_state.current_nesting_depth;
    }
#ifndef OSTRUT_NO_RECURSION_ABORTION
    ostrut_state.callstack_pop_back(&c);
#endif // ifdef OSTRUT_NO_RECURSION_ABORTION
    return out << "}";
  }
  
  // Make pairs ostream-able
  template<typename first_t, typename second_t, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const std::pair<first_t,second_t> &p)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
					      decltype(out << p.first << p.second)>::type
  {
    return ::ostrut::detail::introspect<decltype(p.second)>::
      repr(::ostrut::detail::introspect<decltype(p.first)>::
	   repr(out << "(", p.first) << ", ", p.second) << ")";
  }

  // Make tuples ostream-able. Note that tuples are of bounded length
  // and do not obey expansion limitation defined by ostrut::expansion_limit(n).
  template<typename... types, typename char_t, typename _traits>
    auto operator<<(std::basic_ostream<char_t, _traits> &out, const std::tuple<types...> &tup)
    -> typename ::ostrut::detail::enable_if_all<std::basic_ostream<char_t, _traits> &,
					      decltype(::ostrut::detail::introspect_tuple<std::tuple_size<std::tuple<types...>>
						       ::value>::repr(out, tup))>::type
  {
    using tuple_t = std::tuple<types...>;
    return ::ostrut::detail::introspect_tuple<std::tuple_size<tuple_t>::value>::repr(out << "(", tup) << ")";
  }

  // Make ordinary pointers ostream-able
  template<typename type, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const type *p)
    -> std::basic_ostream<char_t, _traits> & //decltype(out << *p)
  {
    if(p == nullptr) {
      auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
      return out << ostrut_state.nullptr_string();
    }
    return out << *p;
  }
  
  // Make smart pointers ostream-able
  template<typename type, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const type &p)
    -> typename std::enable_if<::ostrut::detail::is_smart_pointer<type>::value,
                               decltype(out << p.get())>::type
  {
    // Just forward to ordinary pointer streaming above.
    return out << p.get();
  }

  // Make weak pointers ostream-able
  template<typename type, typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const type &p)
    -> typename std::enable_if<::ostrut::detail::is_weak_pointer<type>::value,
			       decltype(out << p.lock())>::type
  {
    // Just forward to shared ptr streaming above.
    return out << p.lock();
  }
  
} // namespace std


// # I/O-stream manipulator details
namespace ostrut {
  namespace manip {
    namespace detail {
    
      // A counter (incrementable and decrementable) with a designated
      // value, which is fixed. Bit hacky, e.g., implicit conversion
      // loses knowledge of the fixed-point, but it's okay for our purposes.
      template<typename base_type, base_type fixpoint__>
      struct fixpoint_counter {
	fixpoint_counter() = delete;
      
	fixpoint_counter(const base_type &z)
	  : x_(z)
	{
	}

	fixpoint_counter(const infinite_t &)
	  : x_(fixpoint_)
	{
	}
	
	template <base_type fixpoint2>
	fixpoint_counter &operator=(const fixpoint_counter<base_type, fixpoint2> &z) {
	  static_assert(fixpoint() == fixpoint2, "Mismatch in constant fixpoint, assignment not possible!");
	  x_ = z.x_;
	  return *this;
	}
	
	fixpoint_counter &operator=(const base_type &z) {
	  x_ = z;
	  return *this;
	}
	
	operator base_type &() {
	  return x_;
	}
      
	operator const base_type &() const {
	  return x_;
	}
      
	constexpr base_type fixpoint() const { return fixpoint_; }
      
	bool operator==(const base_type &z) {
	  return x_ == z;
	}
      
	auto operator++() -> typename std::remove_pointer<decltype(this)>::type & {
	  inc_();
	  return *this;
	}
      
	auto operator++(int) -> typename std::remove_pointer<decltype(this)>::type {
	  typename std::remove_pointer<decltype(this)>::type copy(*this);
	  inc_();
	  return copy;
	}
      
	auto operator--() -> typename std::remove_pointer<decltype(this)>::type & {
	  dec_();
	  return *this;
	}
      
	auto operator--(int) -> typename std::remove_pointer<decltype(this)>::type {
	  typename std::remove_pointer<decltype(this)>::type copy(*this);
	  dec_();
	  return copy;
	}
      
      private:
	void inc_() {
	  if(x_ != fixpoint()) {
	    ++x_;
	    assert(x_ != fixpoint());
	  }
	}
	void dec_() {
	  if(x_ != fixpoint()) {
	    --x_;
	    assert(x_ != fixpoint());
	  }
	}
      
	base_type x_;
	static const base_type fixpoint_ = fixpoint__;
      };
    
      struct state_facet
	: std::locale::facet
      {
	static std::locale::id id;
	
	state_facet(size_t refs = 0)
	  : std::locale::facet(refs),
	    current_nesting_depth(0)
	{
	}
      	
	detail::fixpoint_counter<size_t,std::numeric_limits<size_t>::max()> matryoshka_limit() const {
	  return current_settings_.matryoshka_limit_;
	}
	
	void set_matryoshka_limit(size_t n) {
	  current_settings_.matryoshka_limit_ = n;
	}
	
	void set_matryoshka_limit(infinite_t) {
	    current_settings_.matryoshka_limit_ = current_settings_.matryoshka_limit_.fixpoint();
	}

	detail::fixpoint_counter<size_t,std::numeric_limits<size_t>::max()> expansion_limit() const {
	  return current_settings_.expansion_limit_;
	}
	
	void set_expansion_limit(size_t n) {
	  current_settings_.expansion_limit_ = n;
	}
	
	void set_expansion_limit(infinite_t) {
	  current_settings_.expansion_limit_ = current_settings_.expansion_limit_.fixpoint();
	}
	
	const std::string &expansion_canceled_string() const {
	  return current_settings_.expansion_canceled_string_;
	}

	void set_expansion_canceled_string(const std::string &str) {
	  current_settings_.expansion_canceled_string_ = str;
	}
	
	const std::string &nesting_canceled_string() const {
	  return current_settings_.nesting_canceled_string_;
	}

	void set_nesting_canceled_string(const std::string &str) {
	  current_settings_.nesting_canceled_string_ = str;
	}
		
	const std::string &nullptr_string() const {
	  return current_settings_.nullptr_string_;
	}

	void set_nullptr_string(const std::string &str) {
	  current_settings_.nullptr_string_ = str;
	}
		
#ifndef OSTRUT_NO_SETTINGS_STACK
	void push_settings() {
	  settings_stack_.push_back(current_settings_);
	}
	
	void pop_settings(size_t n) {
	  if(n > 1)
	    settings_stack_.erase(settings_stack_.end() - (n-1), settings_stack_.end());
	  current_settings_ = settings_stack_.back();
	  settings_stack_.pop_back();
	}
#endif // ifndef OSTRUT_NO_SETTINGS_STACK

#ifndef OSTRUT_NO_RECURSION_ABORTION
	const std::string &recursion_abortion_string() const {
	  return current_settings_.recursion_abortion_string_;
	}

	void set_recursion_abortion_string(const std::string &str) {
	  current_settings_.recursion_abortion_string_ = str;
	}
	
	template <typename type>
	bool callstack_contains(const type *c)
	{
	  // Do by foot, avoids to pull <algorithm>
	  //std::find(std::begin(callstack_), std::end(callstack_), static_cast<const void *>(c));
	  for(auto x: callstack_)
	    if(x == static_cast<const void *>(c))
	      return true;
	  return false;
	}
	
	template <typename type>
	void callstack_push_back(const type *c)
	{
	  callstack_.push_back(static_cast<const void *>(c));
	}
	
	// asserts that the popped address is c (if != nullptr).
	template <typename type>
	void callstack_pop_back(const type *c = nullptr)
	{
	  if(c != nullptr)
	    assert(callstack_.back() == static_cast<const void *>(c));
	  callstack_.pop_back();
	}
#endif // ifndef OSTRUT_NO_RECURSION_ABORTION
	
      private:
	struct settings_t {
	  settings_t()
	    : expansion_limit_(OSTRUT_DEFAULT_EXPANSION_LIMIT),
	      matryoshka_limit_(OSTRUT_DEFAULT_NESTING_LIMIT),
	      expansion_canceled_string_(OSTRUT_DEFAULT_EXPANSION_CANCELED_STRING),
	      nesting_canceled_string_(OSTRUT_DEFAULT_NESTING_CANCELED_STRING),
#ifndef OSTRUT_NO_RECURSION_ABORTION
	      recursion_abortion_string_(OSTRUT_DEFAULT_RECURSION_ABORTION_STRING),
#endif // OSTRUT_NO_RECURSION_ABORTION
	      nullptr_string_(OSTRUT_DEFAULT_NULLPTR_STRING)
	  { }
	  
	  settings_t &operator=(const settings_t &o) = default;
	  
	  detail::fixpoint_counter<size_t,std::numeric_limits<size_t>::max()> expansion_limit_;
	  detail::fixpoint_counter<size_t,std::numeric_limits<size_t>::max()> matryoshka_limit_;
	  std::string expansion_canceled_string_;
	  std::string nesting_canceled_string_;
#ifndef OSTRUT_NO_RECURSION_ABORTION
	  std::string recursion_abortion_string_;
#endif // OSTRUT_NO_RECURSION_ABORTION
	  std::string nullptr_string_;
	} current_settings_;
#ifndef OSTRUT_NO_SETTINGS_STACK
	std::vector<settings_t> settings_stack_;
#endif // ifndef OSTRUT_NO_SETTINGS_STACK
#ifndef OSTRUT_NO_RECURSION_ABORTION
	std::vector<const void *> callstack_;
#endif // OSTRUT_NO_RECURSION_ABORTION

      public:
	size_t current_nesting_depth;
      };
  
      std::locale::id state_facet::id;
      
      struct expansion_limit_manipulator {
	expansion_limit_manipulator(size_t limit_) : limit(limit_) { }
	size_t limit;
      };
      
      struct expansion_unbounded_manipulator {
	expansion_unbounded_manipulator(infinite_t) { }
      };
      
      struct nesting_limit_manipulator {
	nesting_limit_manipulator(size_t limit_) : limit(limit_) { }
	size_t limit;
      };
       
      struct nesting_unbounded_manipulator {
	nesting_unbounded_manipulator(infinite_t) { }
      };
      
#ifndef OSTRUT_NO_RECURSION_ABORTION
      struct recursion_abortion_string_manipulator {
	recursion_abortion_string_manipulator(const std::string &rec_abor_string_) : recursion_abortion_string(rec_abor_string_) { }
	std::string recursion_abortion_string;
      };
#endif // ifndef OSTRUT_NO_RECURSION_ABORTION
      
      struct nullptr_string_manipulator {
	nullptr_string_manipulator(const std::string &nullptr_string_) : nullptr_string(nullptr_string_) { }
	std::string nullptr_string;
      };
      
#ifndef OSTRUT_NO_SETTINGS_STACK
      struct push_settings_manipulator {
      };
      
      struct pop_settings_manipulator {
	pop_settings_manipulator(size_t n_)
	  : n(n_)
	{}
	size_t n;
      };
#endif // ifndef OSTRUT_NO_SETTINGS_STACK
      
      template<typename char_t, typename _traits>
      const state_facet &getimbue_state_facet(const std::basic_ostream<char_t, _traits> &out) {
	if(!std::has_facet<::ostrut::manip::detail::state_facet>(out.getloc())) {
	  const_cast<std::basic_ostream<char_t, _traits> &>(out).
	    imbue(std::locale(out.getloc(), new ::ostrut::manip::detail::state_facet()));
	}
	assert(std::has_facet<::ostrut::manip::detail::state_facet>(out.getloc()));
	return std::use_facet<::ostrut::manip::detail::state_facet>(out.getloc());
      }
      template<typename char_t, typename _traits>
      state_facet &getimbue_state_facet(std::basic_ostream<char_t, _traits> &out) {
	auto &state = getimbue_state_facet(const_cast<const std::basic_ostream<char_t, _traits> &>(out));
	// Hack! But how else can a non-const reference to the facet be retrieved?
	return const_cast<::ostrut::manip::detail::state_facet &>(state);
      }
      
    } // namespace detail
  } // namespace manip
} // namespace ostrut


// # I/O-stream manipulator user interface
namespace ostrut {

  /// Set maximum expansion depth.  Use like std::cout << ostrut::expansion_limit(n) to set the limit to n or 
  /// like std::cout << ostrut::expansion_limit(infinite) for unlimited expansion.
  auto expansion_limit(size_t new_limit) -> manip::detail::expansion_limit_manipulator
  {
    return manip::detail::expansion_limit_manipulator(new_limit);
  }
  auto expansion_limit(manip::detail::infinite_t inf) -> manip::detail::expansion_unbounded_manipulator
  {
    return manip::detail::expansion_unbounded_manipulator(inf);
  }
  
  /// Set maximum nesting depth.  Use like std::cout << ostrut::nesting_limit(n) to set the limit to n or 
  /// like std::cout << ostrut::nesting_limit(infinite) for unlimited nesting expansion.
  auto nesting_limit(size_t new_limit) -> manip::detail::nesting_limit_manipulator
  {
    return manip::detail::nesting_limit_manipulator(new_limit);
  }
  auto nesting_limit(manip::detail::infinite_t inf) -> manip::detail::nesting_unbounded_manipulator
  {
    return manip::detail::nesting_unbounded_manipulator(inf);
  }
  
  /// Synonym for ostrut::nesting_limit(size_t)
  auto matryoshka_limit(size_t new_limit) -> manip::detail::nesting_limit_manipulator
  {
    return nesting_limit(new_limit);
  }
  auto matryoshka_limit(manip::detail::infinite_t inf) -> manip::detail::nesting_unbounded_manipulator
  {
    return nesting_limit(inf);
  }
  
#ifndef OSTRUT_NO_RECURSION_ABORTION
  auto recursion_abortion_string(const std::string &rec_abort_str) -> manip::detail::recursion_abortion_string_manipulator
  {
    return manip::detail::recursion_abortion_string_manipulator(rec_abort_str);
  }
#endif // ifndef OSTRUT_NO_RECURSION_ABORTION
  
  auto nullptr_string(const std::string &nullptr_str) -> manip::detail::nullptr_string_manipulator
  {
    return manip::detail::nullptr_string_manipulator(nullptr_str);
  }
  
#ifndef OSTRUT_NO_SETTINGS_STACK
  auto push_settings() -> manip::detail::push_settings_manipulator
  {
    return manip::detail::push_settings_manipulator();
  }
  
  auto pop_settings(size_t n = 1) -> manip::detail::pop_settings_manipulator
  {
    return manip::detail::pop_settings_manipulator(n);
  }
#endif // ifndef OSTRUT_NO_SETTINGS_STACK

} // namespace ostrut


// # Further I/O-manipulator details: Inject manipulator handling into std
namespace std {
  
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::expansion_limit_manipulator &exp_manip)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_expansion_limit(exp_manip.limit);
    return out;
  }
  
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::expansion_unbounded_manipulator &)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_expansion_limit(::ostrut::infinite);
    return out;
  }
  
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::nesting_limit_manipulator &exp_manip)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_matryoshka_limit(exp_manip.limit);
    return out;
  }

  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::nesting_unbounded_manipulator &)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_matryoshka_limit(::ostrut::infinite);
    return out;
  }
  
#ifndef OSTRUT_NO_RECURSION_ABORTION
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::recursion_abortion_string_manipulator &manip)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_recursion_abortion_string(manip.recursion_abortion_string);
    return out;
  }
#endif // ifndef OSTRUT_NO_RECURSION_ABORTION
  
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::nullptr_string_manipulator &manip)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.set_nullptr_string(manip.nullptr_string);
    return out;
  }
  
#ifndef OSTRUT_NO_SETTINGS_STACK
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::push_settings_manipulator &)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.push_settings();
    return out;
  }
    
  template<typename char_t, typename _traits>
  auto operator<<(std::basic_ostream<char_t, _traits> &out, const ::ostrut::manip::detail::pop_settings_manipulator &manip)
    -> std::basic_ostream<char_t, _traits> &
  {
    auto &ostrut_state = ::ostrut::manip::detail::getimbue_state_facet(out);
    ostrut_state.pop_settings(manip.n);
    return out;
  }
#endif // ifndef OSTRUT_NO_SETTINGS_STACK
  
} // namespace std

#endif // OSTREAM_UTILITIES_HPP_
