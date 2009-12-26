//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_FA5836A2CADA11DC8CD47C8555D89593
#define UUID_FA5836A2CADA11DC8CD47C8555D89593

#include <boost/config.hpp>
#ifdef BOOST_NO_EXCEPTIONS
#error This header requires exception handling to be enabled.
#endif
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/detail/type_info.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <new>
#include <ios>

namespace
boost
    {
#ifndef BOOST_NO_RTTI
    typedef error_info<struct tag_original_exception_type,std::type_info const *> original_exception_type;

    inline
    std::string
    to_string( original_exception_type const & x )
        {
        return x.value()->name();
        }
#endif

    class exception_ptr;
    exception_ptr current_exception();
    void rethrow_exception( exception_ptr const & );

    class
    exception_ptr:
        public exception_detail::exception_ptr_base
        {
        typedef bool exception_ptr::*unspecified_bool_type;
        friend exception_ptr current_exception();
        friend void rethrow_exception( exception_ptr const & );

        shared_ptr<exception_detail::clone_base const> c_;
        bool bad_alloc_;

        struct
        bad_alloc_tag
            {
            };

        explicit
        exception_ptr( bad_alloc_tag ):
            bad_alloc_(true)
            {
            }

        explicit
        exception_ptr( shared_ptr<exception_detail::clone_base const> const & c ):
            c_(c),
            bad_alloc_(false)
            {
            BOOST_ASSERT(c);
            }

        void
        _rethrow() const
            {
            BOOST_ASSERT(*this);
            if( bad_alloc_ )
                throw enable_current_exception(std::bad_alloc());
            else
                c_->rethrow();
            }

        bool
        _empty() const
            {
            return !bad_alloc_ && !c_;
            }

        public:

        exception_ptr():
            bad_alloc_(false)
            {
            }

        operator unspecified_bool_type() const
            {
            return _empty() ? 0 : &exception_ptr::bad_alloc_;
            }

        friend
        bool
        operator==( exception_ptr const & a, exception_ptr const & b )
            {
            return a.c_==b.c_ && a.bad_alloc_==b.bad_alloc_;
            }

        friend
        bool
        operator!=( exception_ptr const & a, exception_ptr const & b )
            {
            return !(a==b);
            }
        };

    class
    unknown_exception:
        public exception,
        public std::exception,
        public exception_detail::clone_base
        {
        public:

        unknown_exception()
            {
            }

        explicit
        unknown_exception( std::exception const & e )
            {
            add_original_type(e);
            }

        explicit
        unknown_exception( boost::exception const & e ):
            boost::exception(e)
            {
            add_original_type(e);
            }

        ~unknown_exception() throw()
            {
            }

        private:

        exception_detail::clone_base const *
        clone() const
            {
            return new unknown_exception(*this);
            }

        void
        rethrow() const
            {
            throw*this;
            }

        template <class E>
        void
        add_original_type( E const & e )
            {
#ifndef BOOST_NO_RTTI
            (*this) << original_exception_type(&typeid(e));
#endif
            }
        };

    namespace
    exception_detail
        {
        template <class T>
        class
        current_exception_std_exception_wrapper:
            public T,
            public boost::exception,
            public clone_base
            {
            public:

            explicit
            current_exception_std_exception_wrapper( T const & e1 ):
                T(e1)
                {
                add_original_type(e1);
                }

            current_exception_std_exception_wrapper( T const & e1, boost::exception const & e2 ):
                T(e1),
                boost::exception(e2)
                {
                add_original_type(e1);
                }

            ~current_exception_std_exception_wrapper() throw()
                {
                }

            private:

            clone_base const *
            clone() const
                {
                return new current_exception_std_exception_wrapper(*this);
                }

            void
            rethrow() const
                {
                throw *this;
                }

            template <class E>
            void
            add_original_type( E const & e )
                {
#ifndef BOOST_NO_RTTI
                (*this) << original_exception_type(&typeid(e));
#endif
                }
            };

#ifdef BOOST_NO_RTTI
        template <class T>
        exception const *
        get_boost_exception( T const * )
            {
            try
                {
                throw;
                }
            catch(
            exception & x )
                {
                return &x;
                }
            catch(...)
                {
                return 0;
                }
            }
#else
        template <class T>
        exception const *
        get_boost_exception( T const * x )
            {
            return dynamic_cast<exception const *>(x);
            }
#endif

        template <class T>
        inline
        shared_ptr<clone_base const>
        current_exception_std_exception( T const & e1 )
            {
            if( boost::exception const * e2 = get_boost_exception(&e1) )
                return shared_ptr<current_exception_std_exception_wrapper<T> const>(new current_exception_std_exception_wrapper<T>(e1,*e2));
            else
                return shared_ptr<current_exception_std_exception_wrapper<T> const>(new current_exception_std_exception_wrapper<T>(e1));
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_exception()
            {
            return shared_ptr<unknown_exception const>(new unknown_exception());
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_boost_exception( boost::exception const & e )
            {
            return shared_ptr<unknown_exception const>(new unknown_exception(e));
            }

        inline
        shared_ptr<clone_base const>
        current_exception_unknown_std_exception( std::exception const & e )
            {
            if( boost::exception const * be = get_boost_exception(&e) )
                return current_exception_unknown_boost_exception(*be);
            else
                return shared_ptr<unknown_exception const>(new unknown_exception(e));
            }

        inline
        shared_ptr<clone_base const>
        current_exception_impl()
            {
            try
                {
                throw;
                }
            catch(
            exception_detail::clone_base & e )
                {
                return shared_ptr<exception_detail::clone_base const>(e.clone());
                }
            catch(
            std::domain_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::invalid_argument & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::length_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::out_of_range & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::logic_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::range_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::overflow_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::underflow_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::ios_base::failure & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::runtime_error & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::bad_alloc & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
#ifndef BOOST_NO_TYPEID
            catch(
            std::bad_cast & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::bad_typeid & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
#endif
            catch(
            std::bad_exception & e )
                {
                return exception_detail::current_exception_std_exception(e);
                }
            catch(
            std::exception & e )
                {
                return exception_detail::current_exception_unknown_std_exception(e);
                }
            catch(
            boost::exception & e )
                {
                return exception_detail::current_exception_unknown_boost_exception(e);
                }
            catch(
            ... )
                {
                return exception_detail::current_exception_unknown_exception();
                }
            }
        }

    inline
    exception_ptr
    current_exception()
        {
        try
            {
            return exception_ptr(exception_detail::current_exception_impl());
            }
        catch(
        std::bad_alloc & )
            {
            }
        catch(
        ... )
            {
            try
                {
                return exception_ptr(exception_detail::current_exception_std_exception(std::bad_exception()));
                }
            catch(
            std::bad_alloc & )
                {
                }
            catch(
            ... )
                {
                BOOST_ASSERT(0);
                }
            }
        return exception_ptr(exception_ptr::bad_alloc_tag());
        }

    template <class T>
    inline
    exception_ptr
    copy_exception( T const & e )
        {
        try
            {
            throw enable_current_exception(e);
            }
        catch(
        ... )
            {
            return current_exception();
            }
        }

    inline
    void
    rethrow_exception( exception_ptr const & p )
        {
        p._rethrow();
        }

    inline
    std::string
    to_string( exception_ptr const & p )
        {
        std::string s='\n'+diagnostic_information(p);
        std::string padding("  ");
        std::string r;
        bool f=false;
        for( std::string::const_iterator i=s.begin(),e=s.end(); i!=e; ++i )
            {
            if( f )
                r+=padding;
            char c=*i;
            r+=c;
            f=(c=='\n');
            }
        return r;
        }
    }

#endif
