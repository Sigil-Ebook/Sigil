//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_C3E1741C754311DDB2834CCA55D89593
#define UUID_C3E1741C754311DDB2834CCA55D89593

#include <boost/detail/sp_typeinfo.hpp>
#include <boost/current_function.hpp>

namespace
boost
    {
    template <class T>
    inline
    char const *
    tag_type_name()
        {
#ifdef BOOST_NO_TYPEID
        return BOOST_CURRENT_FUNCTION;
#else
        return typeid(T*).name();
#endif
        }

    template <class T>
    inline
    char const *
    type_name()
        {
#ifdef BOOST_NO_TYPEID
        return BOOST_CURRENT_FUNCTION;
#else
        return typeid(T).name();
#endif
        }

    namespace
    exception_detail
        {
#ifdef BOOST_NO_TYPEID
        struct
        type_info_
            {
            detail::sp_typeinfo type_;
            char const * name_;

            explicit
            type_info_( detail::sp_typeinfo type, char const * name ):
                type_(type),
                name_(name)
                {
                }

            friend
            bool
            operator==( type_info_ const & a, type_info_ const & b )
                {
                return a.type_==b.type_;
                }

            friend
            bool
            operator<( type_info_ const & a, type_info_ const & b )
                {
                return a.type_<b.type_;
                }

            char const *
            name() const
                {
                return name_;
                }
            };
#else
        struct
        type_info_
            {
            detail::sp_typeinfo const * type_;

            explicit
            type_info_( detail::sp_typeinfo const & type ):
                type_(&type)
                {
                }

            type_info_( detail::sp_typeinfo const & type, char const * ):
                type_(&type)
                {
                }

            friend
            bool
            operator==( type_info_ const & a, type_info_ const & b )
                {
                return (*a.type_)==(*b.type_);
                }

            friend
            bool
            operator<( type_info_ const & a, type_info_ const & b )
                {
                return 0!=(a.type_->before(*b.type_));
                }

            char const *
            name() const
                {
                return type_->name();
                }
            };
#endif

        inline
        bool
        operator!=( type_info_ const & a, type_info_ const & b )
            {
            return !(a==b);
            }
        }
    }

#define BOOST_EXCEPTION_STATIC_TYPEID(T) ::boost::exception_detail::type_info_(BOOST_SP_TYPEID(T),::boost::tag_type_name<T>())

#ifndef BOOST_NO_RTTI
#define BOOST_EXCEPTION_DYNAMIC_TYPEID(x) ::boost::exception_detail::type_info_(typeid(x))
#endif

#endif
