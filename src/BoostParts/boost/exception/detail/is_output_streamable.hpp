//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_898984B4076411DD973EDFA055D89593
#define UUID_898984B4076411DD973EDFA055D89593

#include <ostream>

namespace
boost
    {
    namespace
    to_string_detail
        {
        template <class T,class CharT,class Traits>
        char operator<<( std::basic_ostream<CharT,Traits> &, T const & );

        template <class T,class CharT,class Traits>
        struct
        is_output_streamable_impl
            {
            static std::basic_ostream<CharT,Traits> & f();
            static T const & g();
            enum e { value=1!=(sizeof(f()<<g())) };
            };
        }

    template <class T, class CharT=char, class Traits=std::char_traits<CharT> >
    struct
    is_output_streamable
        {
        enum e { value=to_string_detail::is_output_streamable_impl<T,CharT,Traits>::value };
        };
    }

#endif
