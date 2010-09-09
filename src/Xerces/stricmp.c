/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: stricmp.c 679398 2008-07-24 12:52:20Z borisk $
 */

#include "stricmp.h"
#include "config.h"

#if HAVE_STRING_H
#	include <string.h>
#endif
#if HAVE_STRINGS_H
#	include <strings.h>
#endif

int stricmp(const char* str1, const char* str2) 
{
#if HAVE_STRCASECMP
	return strcasecmp(str1, str2);
#else
	#error Need implementation of stricmp compatibility function
#endif

}
