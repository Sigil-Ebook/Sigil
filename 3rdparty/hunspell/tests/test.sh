#!/bin/bash
# ##### BEGIN LICENSE BLOCK #####
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# Copyright (C) 2002-2022 Németh László
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Hunspell is based on MySpell which is Copyright (C) 2002 Kevin Hendricks.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
# ##### END LICENSE BLOCK #####

# set -x # uncomment for debugging
set -o pipefail

export LC_ALL="C"

function check_valgrind_log () {
if [[ "$VALGRIND" != "" && -f $TEMPDIR/test.pid* ]]; then
	log=$(ls $TEMPDIR/test.pid*)
	if ! grep -q 'ERROR SUMMARY: 0 error' $log; then
		echo "Fail in $NAME $1 checking detected by Valgrind"
		echo "$log Valgrind log file moved to $TEMPDIR/badlogs"
		mv $log $TEMPDIR/badlogs
		exit 1
	fi
	if grep -q 'LEAK SUMMARY' $log; then
		echo "Memory leak in $NAME $1 checking detected by Valgrind"
		echo "$log Valgrind log file moved to $TEMPDIR/badlogs"
		mv $log $TEMPDIR/badlogs
		exit 1
	fi
	rm -f $log
fi
}

TEMPDIR=./testSubDir
NAME="${1%.dic}"
shift
ENCODING=UTF-8 #io encoding passed with -i
if [[ "$1" == "-i" && -n "$2" ]]; then
	ENCODING="$2"
	shift 2
fi
shopt -s expand_aliases

[[ "$HUNSPELL" = "" ]] && HUNSPELL="$(dirname $0)"/../src/tools/hunspell
[[ "$ANALYZE" = "" ]] && ANALYZE="$(dirname $0)"/../src/tools/analyze
[[ "$LIBTOOL" = "" ]] && LIBTOOL="$(dirname $0)"/../libtool
alias hunspell='"$LIBTOOL" --mode=execute "$HUNSPELL"'
alias analyze='"$LIBTOOL" --mode=execute "$ANALYZE"'

if [[ "$VALGRIND" != "" ]]; then
	mkdir $TEMPDIR 2> /dev/null || :
	rm -f $TEMPDIR/test.pid* || :
	mkdir $TEMPDIR/badlogs 2> /dev/null || :
	alias hunspell='"$LIBTOOL" --mode=execute valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid "$HUNSPELL"'
	alias analyze='"$LIBTOOL" --mode=execute valgrind --tool=$VALGRIND --leak-check=yes --show-reachable=yes --log-file=$TEMPDIR/test.pid "$ANALYZE"'
fi

CR=$(printf "\r")

in_dict="$NAME"
if [[ ! -f "$in_dict.dic" ]]; then
	echo "Dictionary $in_dict.dic does not exists"
	exit 3
fi

# Tests good words
in_file="$in_dict.good"

if [[ -f $in_file ]]; then
	out=$(hunspell -l -i "$ENCODING" "$@" -d "$in_dict" < "$in_file" \
	      | tr -d "$CR")
	if [[ $? -ne 0 ]]; then exit 2; fi
	if [[ "$out" != "" ]]; then
		echo "============================================="
		echo "Fail in $NAME.good. Good words recognised as wrong:"
		echo "$out"
		exit 1
	fi
fi

check_valgrind_log "good words"

# Tests bad words
in_file="$in_dict.wrong"

if [[ -f $in_file ]]; then
	out=$(hunspell -G -i "$ENCODING" "$@" -d "$in_dict" < "$in_file" \
	      | tr -d "$CR") #strip carriage return for mingw builds
	if [[ $? -ne 0 ]]; then exit 2; fi
	if [[ "$out" != "" ]]; then
		echo "============================================="
		echo "Fail in $NAME.wrong. Bad words recognised as good:"
		echo "$out"
		exit 1
	fi
fi

check_valgrind_log "bad words"

# Tests good words' root
in_file="$in_dict.good"
expected_file="$in_dict.root"

if [[ -f $expected_file ]]; then
        # Extract the root words of the affixed words, after '+'
        out=$(hunspell -d "$in_dict" < "$in_file" | grep -a '^+ ' \
              | sed 's/^+ //')
	if [[ $? -ne 0 ]]; then exit 2; fi
	expected=$(<"$expected_file")
	if [[ "$out" != "$expected" ]]; then
		echo "============================================="
		echo "Fail in $NAME.root. Bad analysis?"
		diff "$expected_file" <(echo "$out") | grep '^<' | sed 's/^..//'
		exit 1
	fi
fi

check_valgrind_log "root"

# Tests morphological analysis
in_file="$in_dict.good"
expected_file="$in_dict.morph"

if [[ -f $expected_file ]]; then
	#in=$(sed 's/	$//' "$in_file") #passes without this.
	out=$(analyze "$in_dict.aff" "$in_dict.dic" "$in_file" \
	      | tr -d "$CR") #strip carige return for mingw builds
	if [[ $? -ne 0 ]]; then exit 2; fi
	expected=$(<"$expected_file")
	if [[ "$out" != "$expected" ]]; then
		echo "============================================="
		echo "Fail in $NAME.morph. Bad analysis?"
		diff "$expected_file" <(echo "$out") | grep '^<' | sed 's/^..//'
		exit 1
	fi
fi

check_valgrind_log "morphological analysis"

# Tests suggestions
in_file=$in_dict.wrong
expected_file=$in_dict.sug

if [[ -f $expected_file ]]; then
	out=$(hunspell -i "$ENCODING" "$@" -a -d "$in_dict" <"$in_file" | \
	      { grep -a '^&' || true; } | sed 's/^[^:]*: //')
	if [[ $? -ne 0 ]]; then exit 2; fi
	expected=$(<"$expected_file")
	if [[ "$out" != "$expected" ]]; then
		echo "============================================="
		echo "Fail in $NAME.sug. Bad suggestion?"
		diff "$expected_file" <(echo "$out")
		exit 1
	fi
fi

check_valgrind_log "suggestion"
