//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_CE6983AC753411DDA764247956D89593
#define UUID_CE6983AC753411DDA764247956D89593

#include <string>

namespace
boost
    {
    namespace
    exception_detail
        {
        class
        error_info_base
            {
            public:

            virtual char const * tag_typeid_name() const = 0;
            virtual std::string value_as_string() const = 0;

            protected:

            virtual
            ~error_info_base() throw()
                {
                }
            };
        }

    template <class Tag,class T>
    class
    error_info:
        public exception_detail::error_info_base
        {
        public:

        typedef T value_type;

        error_info( value_type const & value );
        ~error_info() throw();

        value_type const &
        value() const
            {
            return value_;
            }

        value_type &
        value()
            {
            return value_;
            }

        private:

        char const * tag_typeid_name() const;
        std::string value_as_string() const;

        value_type value_;
        };
    }

#endif
