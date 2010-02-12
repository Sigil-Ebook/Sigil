#ifndef BOOST_THREAD_WIN32_ONCE_HPP
#define BOOST_THREAD_WIN32_ONCE_HPP

//  once.hpp
//
//  (C) Copyright 2005-7 Anthony Williams 
//  (C) Copyright 2005 John Maddock
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include <cstddef>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/detail/interlocked.hpp>
#include <boost/thread/win32/thread_primitives.hpp>
#include <boost/thread/win32/interlocked_read.hpp>

#include <boost/config/abi_prefix.hpp>

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
    using ::memcpy;
    using ::ptrdiff_t;
}
#endif

namespace boost
{
    typedef long once_flag;

#define BOOST_ONCE_INIT 0

    namespace detail
    {
        struct win32_mutex_scoped_lock
        {
            void* const mutex_handle;
            explicit win32_mutex_scoped_lock(void* mutex_handle_):
                mutex_handle(mutex_handle_)
            {
                BOOST_VERIFY(!win32::WaitForSingleObject(mutex_handle,win32::infinite));
            }
            ~win32_mutex_scoped_lock()
            {
                BOOST_VERIFY(win32::ReleaseMutex(mutex_handle)!=0);
            }
        private:
            void operator=(win32_mutex_scoped_lock&);
        };

#ifdef BOOST_NO_ANSI_APIS
        template <class I>
        void int_to_string(I p, wchar_t* buf)
        {
            for(unsigned i=0; i < sizeof(I)*2; ++i,++buf)
            {
                *buf = L'A' + static_cast<wchar_t>((p >> (i*4)) & 0x0f);
            }
            *buf = 0;
        }
#else
        template <class I>
        void int_to_string(I p, char* buf)
        {
            for(unsigned i=0; i < sizeof(I)*2; ++i,++buf)
            {
                *buf = 'A' + static_cast<char>((p >> (i*4)) & 0x0f);
            }
            *buf = 0;
        }
#endif

        // create a named mutex. It doesn't really matter what this name is
        // as long as it is unique both to this process, and to the address of "flag":
        inline void* create_once_mutex(void* flag_address)
        {
        
#ifdef BOOST_NO_ANSI_APIS
            typedef wchar_t char_type;
            static const char_type fixed_mutex_name[]=L"{C15730E2-145C-4c5e-B005-3BC753F42475}-once-flag";
#else
            typedef char char_type;
            static const char_type fixed_mutex_name[]="{C15730E2-145C-4c5e-B005-3BC753F42475}-once-flag";
#endif
            unsigned const once_mutex_name_fixed_buffer_size=sizeof(fixed_mutex_name)/sizeof(char_type);
            unsigned const once_mutex_name_fixed_length=once_mutex_name_fixed_buffer_size-1;
            unsigned const once_mutex_name_length=once_mutex_name_fixed_buffer_size+sizeof(void*)*2+sizeof(unsigned long)*2;
            char_type mutex_name[once_mutex_name_length];
            
            std::memcpy(mutex_name,fixed_mutex_name,sizeof(fixed_mutex_name));

            BOOST_STATIC_ASSERT(sizeof(void*) == sizeof(std::ptrdiff_t));
            detail::int_to_string(reinterpret_cast<std::ptrdiff_t>(flag_address), mutex_name + once_mutex_name_fixed_length);
            detail::int_to_string(win32::GetCurrentProcessId(), mutex_name + once_mutex_name_fixed_length + sizeof(void*)*2);

#ifdef BOOST_NO_ANSI_APIS
            return win32::CreateMutexW(0, 0, mutex_name);
#else
            return win32::CreateMutexA(0, 0, mutex_name);
#endif
        }

        
    }
    

    template<typename Function>
    void call_once(once_flag& flag,Function f)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        long const function_complete_flag_value=0xc15730e2;

        if(::boost::detail::interlocked_read_acquire(&flag)!=function_complete_flag_value)
        {
            void* const mutex_handle(::boost::detail::create_once_mutex(&flag));
            BOOST_ASSERT(mutex_handle);
            detail::win32::handle_manager const closer(mutex_handle);
            detail::win32_mutex_scoped_lock const lock(mutex_handle);
      
            if(flag!=function_complete_flag_value)
            {
                f();
                BOOST_INTERLOCKED_EXCHANGE(&flag,function_complete_flag_value);
            }
        }
    }
}

#include <boost/config/abi_suffix.hpp>

#endif
