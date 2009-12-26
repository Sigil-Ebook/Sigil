//Copyright (c) 2006-2009 Emil Dotchevski and Reverge Studios, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_DC4208C6417811DEBF11E1EC55D89593
#define UUID_DC4208C6417811DEBF11E1EC55D89593

namespace
boost
    {
        namespace
        exception_detail
                {
                class
                exception_ptr_base
                        {
                        public:

                        virtual void _rethrow() const=0;
                        virtual bool _empty() const=0;

                        protected:

                        virtual
                        ~exception_ptr_base() throw()
                            {
                            }
                        };
                }
    }

#endif
