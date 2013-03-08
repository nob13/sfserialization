#pragma once
 
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>

#include "types.h"

/**@file
isDefault methods for different types. They are needed for Serialization
so that it can check for unchanged values which do not need to get serialized.
*/

namespace sf {

inline bool isDefault (const int32_t & data){
	return data == 0;
}

inline bool isDefault (const int64_t & data){
	return data == 0;
}

inline bool isDefault (const float & data){
	return data == 0;
}

inline bool isDefault (const double & data){
	return data == 0;
}

inline bool isDefault (const bool & data) {
	return data == false;
}

inline bool isDefault (const std::string & s) {
	return s.empty();
}

template <class T> static bool isDefault (const std::vector<T> & data){
	return data.empty();
}

template <class T> static bool isDefault (const std::set<T> & data){
	return data.empty();
}

template <class A, class B> static bool isDefault (const std::map<A,B> & data){
	return data.empty();
}

// T is enum
template <typename T>
 typename boost::enable_if< boost::is_enum<T>, bool>::type
 isDefault_b (const T & t){
	return t == T();
}

// T is no enum and does not have a isDefault method
template <typename T>
 typename boost::disable_if< boost::is_enum<T>, bool>::type
 isDefault_b (const T & t){
	return false;
}



#ifdef __GNUC__
// SFINAE test whether there is an isDefault method
template <typename T>
class hasIsDefault
{
    typedef char one;
    typedef long two;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    template <typename C> static one test( decltype(&C::isDefault) ) ;
#else
    template <typename C> static one test( typeof(&C::isDefault) ) ;
#endif
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// Type has an isDefault method
template <class T>
 typename boost::enable_if_c< hasIsDefault<T>::value, bool>::type
 isDefault (const T & t){
	return t.isDefault();
}

// Fallback, if no isDefault method available
template <class T>
 typename boost::disable_if_c< hasIsDefault<T>::value, bool>::type
 isDefault (const T & t){
	return isDefault_b (t);
}
#endif
#ifdef _MSC_VER
template <class T>
bool isDefault (const T & t){
	__if_exists (T::isDefault){
		return t.isDefault();
	}
	__if_not_exists (T::isDefault){
		return isDefault_b (t);
	}
}
#endif


}
