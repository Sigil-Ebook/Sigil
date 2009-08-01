
cmake_minimum_required( VERSION 2.6 ) 

project( tidyLib )

set( SOURCES access.c
			access.h
			alloc.c
			attrask.c
			attrdict.c
			attrdict.h
			attrget.c
			attrs.c
			attrs.h
			buffio.c
			buffio.h
			charsets.c
			charsets.h
			clean.c
			clean.h
			config.c
			config.h
			entities.c
			entities.h
			fileio.c
			fileio.h
			forward.h
			iconvtc.c
			iconvtc.h
			istack.c
			lexer.c
			lexer.h
			localize.c
			mappedio.c
			mappedio.h
			message.h
			parser.c
			parser.h
			platform.h
			pprint.c
			pprint.h
			streamio.c
			streamio.h
			tagask.c
			tags.c
			tags.h
			tidy-int.h
			tidy.h
			tidyenum.h
			tidylib.c
			tmbstr.c
			tmbstr.h
			utf8.c
			utf8.h
			version.h
			win32tc.c
			win32tc.h  )
			
add_library( tidyLib ${SOURCES} ) 