#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hunspell],[1.7.2],[https://github.com/hunspell/hunspell/issues],,[https://hunspell.github.io])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE

# the following 4 lines are used for pkg-check's .pc.in file
HUNSPELL_VERSION_MAJOR=`echo $VERSION | cut -d"." -f1`
HUNSPELL_VERSION_MINOR=`echo $VERSION | cut -d"." -f2`
AC_SUBST(HUNSPELL_VERSION_MAJOR)
AC_SUBST(HUNSPELL_VERSION_MINOR)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
LT_INIT
AC_PROG_AWK

AX_CXX_COMPILE_STDCXX_11([noext], [optional])

# Checks for libraries.
AM_ICONV
AC_REQUIRE_AUX_FILE([tap-driver.sh])

# Checks for header files.
AC_CHECK_HEADERS([locale.h langinfo.h])

# Checks for typedefs, structures, and compiler characteristics.
gl_VISIBILITY

# Checks for library functions.

dnl internationalization macros
AM_GNU_GETTEXT_VERSION(0.18)
AM_GNU_GETTEXT([external])

AC_ARG_WITH(warnings,[  --with-warnings         compile with warning messages],[
	AC_DEFINE(HUNSPELL_WARNING_ON,1,"Define if you need warning messages")
])

CURSESLIB=""
AC_ARG_WITH(
	[ui],
	[AS_HELP_STRING([--with-ui],[support Curses user interface])],
	[],
	[with_ui=no]
)
AS_IF([test "x$with_ui" != xno], [
	AC_CHECK_LIB([ncursesw],[tparm],[
		CURSESLIB=-lncursesw
	],[AC_CHECK_LIB([curses],[tparm],[
		CURSESLIB=-lcurses
	],[AC_CHECK_LIB([ncurses],[tparm],[
		CURSESLIB=-lncurses
	])])])
   if test "$CURSESLIB" != "" ; then
	echo Compiling with curses user interface.
	AC_CHECK_HEADERS([curses.h])
	if test "$CURSESLIB" = "-lncursesw" ; then
		AC_CHECK_HEADERS([ncursesw/curses.h])
	else
		echo "No Unicode support on interactive console. (Install Ncursesw library.)"
	fi
	AC_DEFINE(HUNSPELL_WARNING_ON,1,"Define if you need warning messages")
   fi
])
AC_SUBST(CURSESLIB)

AC_ARG_WITH(
	[readline],
	[AS_HELP_STRING([--with-readline],[support fancy command input editing])],
	[],
	[with_readline=no]
)
READLINELIB=""
AS_IF([test "x$with_readline" != xno],
	[AC_CHECK_LIB([tinfo],[tgetent], [
		TERMLIB=-ltinfo
        ],[AC_CHECK_LIB([curses],[tparm],[
		TERMLIB=-lncurses
	],[AC_CHECK_LIB([termcap],[tgetent],[
		TERMLIB=-ltermcap
	])])])
	LDSAVE=$LDFLAGS
	LDFLAGS="$LDFLAGS $TERMLIB"
	AC_CHECK_LIB([readline],[readline],[
		AC_CHECK_HEADER([readline/readline.h],[
			READLINELIB="-lreadline $TERMLIB"
			echo Using the readline library.
			AC_DEFINE([HAVE_READLINE],[1],
			["Define if you have fancy command input editing with Readline"])
		])
	])
])
AC_SUBST(READLINELIB)

AC_CONFIG_FILES([
	Makefile
	hunspell.pc
	man/Makefile
	man/hu/Makefile
	po/Makefile.in
	src/Makefile
	src/hunspell/Makefile
	src/hunspell/hunvisapi.h
	src/parsers/Makefile
	src/tools/Makefile
	tests/Makefile
	tests/suggestiontest/Makefile
])
AC_OUTPUT
