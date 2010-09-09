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
 * $Id: XMLString.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLSTRING_HPP)
#define XERCESC_INCLUDE_GUARD_XMLSTRING_HPP

#include <xercesc/util/BaseRefVectorOf.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <string.h>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

class XMLLCPTranscoder;
/**
  * Class for representing native character strings and handling common string
  * operations
  *
  * This class is Unicode compliant. This class is designed primarily
  * for internal use, but due to popular demand, it is being made
  * publicly available. Users of this class must understand that this
  * is not an officially supported class. All public methods of this
  * class are <i>static functions</i>.
  *
  */
class XMLUTIL_EXPORT XMLString
{
public:
    /* Static methods for native character mode string manipulation */


    /** @name String concatenation functions */
    //@{
    /** Concatenates two strings.
      *
      * <code>catString</code> appends <code>src</code> to <code>target</code> and
      * terminates the resulting string with a null character. The initial character
      * of <code>src</code> overwrites the terminating character of <code>target
      * </code>.
      *
      * No overflow checking is performed when strings are copied or appended.
      * The behavior of <code>catString</code> is undefined if source and
      * destination strings overlap.
      *
      * @param target Null-terminated destination string
      * @param src Null-terminated source string
      */
    static void catString
    (
                char* const     target
        , const char* const     src
    );

    /** Concatenates two strings.
      *
      * <code>catString</code> appends <code>src</code> to <code>target</code> and
      * terminates the resulting string with a null character. The initial character of
      * <code>src</code> overwrites the terminating character of <code>target</code>.
      * No overflow checking is performed when strings are copied or appended.
      * The behavior of <code>catString</code> is undefined if source and destination
      * strings overlap.
      *
      * @param target Null-terminated destination string
      * @param src Null-terminated source string
      */
    static void catString
    (
                XMLCh* const    target
        , const XMLCh* const    src
    );
    //@}

    /** @name String comparison functions */
    //@{
    /** Lexicographically compares lowercase versions of <code>str1</code> and
      * <code>str2</code> and returns a value indicating their relationship.
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      *
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareIString
    (
        const   char* const     str1
        , const char* const     str2
    );

    /** Lexicographically compares lowercase versions of <code>str1</code> and
      * <code>str2</code> and returns a value indicating their relationship.
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareIString
    (
        const   XMLCh* const    str1
        , const XMLCh* const    str2
    );

    /** Lexicographically compares lowercase versions of <code>str1</code> and
      * <code>str2</code> and returns a value indicating their relationship.
      * The routine only lowercases A to Z.
      * @param str1 Null-terminated ASCII string to compare
      * @param str2 Null-terminated ASCII string to compare
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareIStringASCII
    (
        const   XMLCh* const    str1
        , const XMLCh* const    str2
    );



    /** Lexicographically compares, at most, the first count characters in
      * <code>str1</code> and <code>str2</code> and returns a value indicating the
      * relationship between the substrings.
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @param count The number of characters to compare
      *
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareNString
    (
        const   char* const     str1
        , const char* const     str2
        , const XMLSize_t       count
    );

    /** Lexicographically compares, at most, the first count characters in
      * <code>str1</code> and <code>str2</code> and returns a value indicating
      * the relationship between the substrings.
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @param count The number of characters to compare
      *
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareNString
    (
        const   XMLCh* const    str1
        , const XMLCh* const    str2
        , const XMLSize_t       count
    );


    /** Lexicographically compares, at most, the first count characters in
      * <code>str1</code> and <code>str2</code> without regard to case and
      * returns a value indicating the relationship between the substrings.
      *
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @param count The number of characters to compare
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareNIString
    (
        const   char* const     str1
        , const char* const     str2
        , const XMLSize_t       count
    );

    /** Lexicographically compares, at most, the first count characters in
      * <code>str1</code> and <code>str2</code> without regard to case and
      * returns a value indicating the relationship between the substrings.
      *
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @param count The number of characters to compare
      *
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareNIString
    (
        const   XMLCh* const    str1
        , const XMLCh* const    str2
        , const XMLSize_t       count
    );

    /** Lexicographically compares <code>str1</code> and <code>str2</code> and
      * returns a value indicating their relationship.
      *
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      *
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareString
    (
        const   char* const     str1
        , const char* const     str2
    );

    /** Lexicographically compares <code>str1</code> and <code>str2</code> and
      * returns a value indicating their relationship.
      *
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @return The return value indicates the relation of <code>str1</code> to
      * <code>str2</code> as follows
      *  Less than 0 means <code>str1</code> is less than <code>str2</code>
      *  Equal to 0 means <code>str1</code> is identical to <code>str2</code>
      *  Greater than 0 means <code>str1</code> is more than <code>str2</code>
      */
    static int compareString
    (
        const   XMLCh* const    str1
        , const XMLCh* const    str2
    );

    /** compares <code>str1</code> and <code>str2</code>
      *
      * @param str1 Null-terminated string to compare
      * @param str2 Null-terminated string to compare
      * @return true if two strings are equal, false if not
      *  If one string is null, while the other is zero-length string,
      *  it is considered as equal.
      */
    static bool equals
    (
          const XMLCh* str1
        , const XMLCh* str2
    );

    /** compares <code>str1</code> and <code>str2</code>
      *
      * @param str1 string to compare
      * @param str2 string to compare
      * @param n number of characters to compare
      * @return true if two strings are equal, false if not
      *  If one string is null, while the other is zero-length string,
      *  it is considered as equal.
      */
    static bool equalsN
    (
          const XMLCh* str1
        , const XMLCh* str2
        , XMLSize_t n
    );

    static bool equals
    (
          const char* str1
        , const char* str2
    );

    /** compares <code>str1</code> and <code>str2</code>
      *
      * @param str1 string to compare
      * @param str2 string to compare
      * @param n number of characters to compare
      * @return true if two strings are equal, false if not
      *  If one string is null, while the other is zero-length string,
      *  it is considered as equal.
      */
    static bool equalsN
    (
          const char* str1
        , const char* str2
        , XMLSize_t n
    );

	/** Lexicographically compares <code>str1</code> and <code>str2</code>
	  * regions and returns true if they are equal, otherwise false.
	  *
      * A substring of <code>str1</code> is compared to a substring of
	  * <code>str2</code>. The result is true if these substrings represent
	  * identical character sequences. The substring of <code>str1</code>
      * to be compared begins at offset1 and has length charCount. The
	  * substring of <code>str2</code> to be compared begins at offset2 and
	  * has length charCount. The result is false if and only if at least
      * one of the following is true:
      *   offset1 is negative.
      *   offset2 is negative.
      *   offset1+charCount is greater than the length of str1.
      *   offset2+charCount is greater than the length of str2.
      *   There is some nonnegative integer k less than charCount such that:
      *   str1.charAt(offset1+k) != str2.charAt(offset2+k)
      *
      * @param str1 Null-terminated string to compare
	  * @param offset1 Starting offset of str1
      * @param str2 Null-terminated string to compare
	  * @param offset2 Starting offset of str2
	  * @param charCount The number of characters to compare
      * @return true if the specified subregion of <code>str1</code> exactly
	  *  matches the specified subregion of <code>str2></code>; false
	  *  otherwise.
      */
    static bool regionMatches
    (
        const   XMLCh* const    str1
		, const	int				offset1
        , const XMLCh* const    str2
		, const int				offset2
		, const XMLSize_t       charCount
    );

	/** Lexicographically compares <code>str1</code> and <code>str2</code>
	  * regions without regard to case and returns true if they are equal,
	  * otherwise false.
	  *
      * A substring of <code>str1</code> is compared to a substring of
	  * <code>str2</code>. The result is true if these substrings represent
	  * identical character sequences. The substring of <code>str1</code>
      * to be compared begins at offset1 and has length charCount. The
	  * substring of <code>str2</code> to be compared begins at offset2 and
	  * has length charCount. The result is false if and only if at least
      * one of the following is true:
      *   offset1 is negative.
      *   offset2 is negative.
      *   offset1+charCount is greater than the length of str1.
      *   offset2+charCount is greater than the length of str2.
      *   There is some nonnegative integer k less than charCount such that:
      *   str1.charAt(offset1+k) != str2.charAt(offset2+k)
      *
      * @param str1 Null-terminated string to compare
	  * @param offset1 Starting offset of str1
      * @param str2 Null-terminated string to compare
	  * @param offset2 Starting offset of str2
	  * @param charCount The number of characters to compare
      * @return true if the specified subregion of <code>str1</code> exactly
	  *  matches the specified subregion of <code>str2></code>; false
	  *  otherwise.
      */
    static bool regionIMatches
    (
        const   XMLCh* const    str1
		, const	int				offset1
        , const XMLCh* const    str2
		, const int				offset2
		, const XMLSize_t       charCount
    );
    //@}

    /** @name String copy functions */
    //@{
    /** Copies <code>src</code>, including the terminating null character, to the
      * location specified by <code>target</code>.
      *
      * No overflow checking is performed when strings are copied or appended.
      * The behavior of strcpy is undefined if the source and destination strings
      * overlap.
      *
      * @param target Destination string
      * @param src Null-terminated source string
      */
    static void copyString
    (
                char* const     target
        , const char* const     src
    );

    /** Copies <code>src</code>, including the terminating null character, to
      *   the location specified by <code>target</code>.
      *
      * No overflow checking is performed when strings are copied or appended.
      * The behavior of <code>copyString</code> is undefined if the source and
      * destination strings overlap.
      *
      * @param target Destination string
      * @param src Null-terminated source string
      */
    static void copyString
    (
                XMLCh* const    target
        , const XMLCh* const    src
    );

    /** Copies <code>src</code>, upto a fixed number of characters, to the
      * location specified by <code>target</code>.
      *
      * No overflow checking is performed when strings are copied or appended.
      * The behavior of <code>copyNString</code> is undefined if the source and
      * destination strings overlap.
      *
      * @param target Destination string. The size of the buffer should
      *        atleast be 'maxChars + 1'.
      * @param src Null-terminated source string
      * @param maxChars The maximum number of characters to copy
      */
    static bool copyNString
    (
                XMLCh* const   target
        , const XMLCh* const   src
        , const XMLSize_t      maxChars
    );
    //@}

    /** @name Hash functions */
    //@{
    /** Hashes a string given a modulus
      *
      * @param toHash The string to hash
      * @param hashModulus The divisor to be used for hashing
      * @return Returns the hash value
      */
    static XMLSize_t hash
    (
        const   char* const     toHash
        , const XMLSize_t       hashModulus
    );

    /** Hashes a string given a modulus
      *
      * @param toHash The string to hash
      * @param hashModulus The divisor to be used for hashing
      * @return Returns the hash value
      */
    static XMLSize_t hash
    (
        const   XMLCh* const    toHash
        , const XMLSize_t       hashModulus
    );

    /** Hashes a string given a modulus taking a maximum number of characters
      * as the limit
      *
      * @param toHash The string to hash
      * @param numChars The maximum number of characters to consider for hashing
      * @param hashModulus The divisor to be used for hashing
      * @return Returns the hash value
      */
    static XMLSize_t hashN
    (
        const   XMLCh* const    toHash
        , const XMLSize_t       numChars
        , const XMLSize_t       hashModulus
    );

    //@}

    /** @name Search functions */
    //@{
    /**
      * Provides the index of the first occurrence of a character within a string
      *
      * @param toSearch The string to search
      * @param ch The character to search within the string
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int indexOf(const char* const toSearch, const char ch);

    /**
      * Provides the index of the first occurrence of a character within a string
      *
      * @param toSearch The string to search
      * @param ch The character to search within the string
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int indexOf(const XMLCh* const toSearch, const XMLCh ch);

	    /**
      * Provides the index of the first occurrence of a character within a string
      * starting from a given index
      *
      * @param toSearch The string to search
      * @param chToFind The character to search within the string
      * @param fromIndex The index to start searching from
      * @param manager The MemoryManager to use to allocate objects
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int indexOf
    (
        const   char* const     toSearch
        , const char            chToFind
        , const XMLSize_t       fromIndex
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Provides the index of the first occurrence of a character within a string
      * starting from a given index
      *
      * @param toSearch The string to search
      * @param chToFind The character to search within the string
      * @param fromIndex The index to start searching from
      * @param manager The MemoryManager to use to allocate objects
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int indexOf
    (
        const   XMLCh* const    toSearch
        , const XMLCh           chToFind
        , const XMLSize_t       fromIndex
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Provides the index of the last occurrence of a character within a string
      *
      * @param toSearch The string to search
      * @param ch The character to search within the string
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int lastIndexOf(const char* const toSearch, const char ch);

    /**
      * Provides the index of the last occurrence of a character within a string
      *
      * @param toSearch The string to search
      * @param ch The character to search within the string
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int lastIndexOf(const XMLCh* const toSearch, const XMLCh ch);

    /**
      * Provides the index of the last occurrence of a character within a string
      *
      * @param ch The character to search within the string
      * @param toSearch The string to search
      * @param toSearchLen The length of the string to search
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int lastIndexOf
    (
        const XMLCh ch
        , const XMLCh* const toSearch
        , const XMLSize_t    toSearchLen
    );

    /**
      * Provides the index of the last occurrence of a character within a string
      * starting backward from a given index
      *
      * @param toSearch The string to search
      * @param chToFind The character to search within the string
      * @param fromIndex The index to start backward search from
      * @param manager The MemoryManager to use to allocate objects
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int lastIndexOf
    (
        const   char* const     toSearch
        , const char            chToFind
        , const XMLSize_t       fromIndex
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Provides the index of the last occurrence of a character within a string
      * starting backward from a given index
      *
      * @param toSearch The string to search
      * @param ch       The character to search within the string
      * @param fromIndex The index to start backward search from
      * @param manager The MemoryManager to use to allocate objects
      * @return If found, returns the index of the character within the string,
      * else returns -1.
      */
    static int lastIndexOf
    (
        const   XMLCh* const    toSearch
        , const XMLCh           ch
        , const XMLSize_t       fromIndex
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    //@}

    /** @name Fixed size string movement */
    //@{
    /** Moves X number of chars
      * @param targetStr The string to copy the chars to
      * @param srcStr The string to copy the chars from
      * @param count The number of chars to move
      */
    static void moveChars
    (
                XMLCh* const    targetStr
        , const XMLCh* const    srcStr
        , const XMLSize_t       count
    );

    //@}

    /** @name Substring function */
    //@{
    /** Create a substring of a given string. The substring begins at the
      * specified beginIndex and extends to the character at index
      * endIndex - 1.
      * @param targetStr The string to copy the chars to
      * @param srcStr The string to copy the chars from
      * @param startIndex beginning index, inclusive.
      * @param endIndex the ending index, exclusive.
      * @param manager The MemoryManager to use to allocate objects
      */
    static void subString
    (
                char* const    targetStr
        , const char* const    srcStr
        , const XMLSize_t      startIndex
        , const XMLSize_t      endIndex
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Create a substring of a given string. The substring begins at the
      * specified beginIndex and extends to the character at index
      * endIndex - 1.
      * @param targetStr The string to copy the chars to
      * @param srcStr The string to copy the chars from
      * @param startIndex beginning index, inclusive.
      * @param endIndex the ending index, exclusive.
      * @param manager The MemoryManager to use to allocate objects
      */
    static void subString
    (
                XMLCh* const    targetStr
        , const XMLCh* const    srcStr
        , const XMLSize_t       startIndex
        , const XMLSize_t       endIndex
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Create a substring of a given string. The substring begins at the
      * specified beginIndex and extends to the character at index
      * endIndex - 1.
      * @param targetStr The string to copy the chars to
      * @param srcStr The string to copy the chars from
      * @param startIndex beginning index, inclusive.
      * @param endIndex the ending index, exclusive.
      * @param srcStrLength the length of srcStr
      * @param manager The MemoryManager to use to allocate objects
      */
    static void subString
    (
                XMLCh* const    targetStr
        , const XMLCh* const    srcStr
        , const XMLSize_t       startIndex
        , const XMLSize_t       endIndex
        , const XMLSize_t       srcStrLength
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /** @name Replication function */
    //@{
    /** Replicates a string
      * NOTE: The returned buffer is allocated with the MemoryManager. It is the
      * responsibility of the caller to delete it when not longer needed.
      * You can call XMLString::release to release this returned buffer.
      *
      * @param toRep The string to replicate
      * @param manager The MemoryManager to use to allocate the string
      * @return Returns a pointer to the replicated string
      * @see   XMLString::release(char**, MemoryManager*)
      */
    static char* replicate(const char* const toRep,
                           MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Replicates a string
      * NOTE: The returned buffer is allocated with the MemoryManager. It is the
      * responsibility of the caller to delete it when not longer needed.
      * You can call XMLString::release to release this returned buffer.
      *
      * @param toRep The string to replicate
      * @param manager The MemoryManager to use to allocate the string
      * @return Returns a pointer to the replicated string
      * @see   XMLString::release(XMLCh**, MemoryManager*)
      */
    static XMLCh* replicate(const XMLCh* const toRep,
                            MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@}

    /** @name String query function */
    //@{
    /** Tells if the sub-string appears within a string at the beginning
      * @param toTest The string to test
      * @param prefix The sub-string that needs to be checked
      * @return Returns true if the sub-string was found at the beginning of
      * <code>toTest</code>, else false
      */
    static bool startsWith
    (
        const   char* const     toTest
        , const char* const     prefix
    );

    /** Tells if the sub-string appears within a string at the beginning
      * @param toTest The string to test
      * @param prefix The sub-string that needs to be checked
      * @return Returns true if the sub-string was found at the beginning of
      * <code>toTest</code>, else false
      */
    static bool startsWith
    (
        const   XMLCh* const    toTest
        , const XMLCh* const    prefix
    );

    /** Tells if the sub-string appears within a string at the beginning
      * without regard to case
      *
      * @param toTest The string to test
      * @param prefix The sub-string that needs to be checked
      * @return Returns true if the sub-string was found at the beginning of
      * <code>toTest</code>, else false
      */
    static bool startsWithI
    (
        const   char* const     toTest
        , const char* const     prefix
    );

    /** Tells if the sub-string appears within a string at the beginning
      * without regard to case
      *
      * @param toTest The string to test
      * @param prefix The sub-string that needs to be checked
      *
      * @return Returns true if the sub-string was found at the beginning
      * of <code>toTest</code>, else false
      */
    static bool startsWithI
    (
        const   XMLCh* const    toTest
        , const XMLCh* const    prefix
    );

    /** Tells if the sub-string appears within a string at the end.
      * @param toTest The string to test
      * @param suffix The sub-string that needs to be checked
      * @return Returns true if the sub-string was found at the end of
      * <code>toTest</code>, else false
      */
    static bool endsWith
    (
        const   XMLCh* const    toTest
        , const XMLCh* const    suffix
    );


    /** Tells if a string has any occurrence of any character of another
      * string within itself
      * @param toSearch The string to be searched
      * @param searchList The string from which characters to be searched for are drawn
      * @return Returns the pointer to the location where the first occurrence of any
      * character from searchList is found,
      * else returns 0
      */
    static const XMLCh* findAny
    (
        const   XMLCh* const    toSearch
        , const XMLCh* const    searchList
    );

    /** Tells if a string has any occurrence of any character of another
      * string within itself
      * @param toSearch The string to be searched
      * @param searchList The string from which characters to be searched for are drawn
      * @return Returns the pointer to the location where the first occurrence of any
      * character from searchList is found,
      * else returns 0
      */
    static XMLCh* findAny
    (
                XMLCh* const    toSearch
        , const XMLCh* const    searchList
    );

    /** Tells if a string has pattern within itself
      * @param toSearch The string to be searched
      * @param pattern The pattern to be located within the string
      * @return Returns index to the location where the pattern was
      * found, else returns -1
      */
    static int patternMatch
    (
          const XMLCh* const    toSearch
        , const XMLCh* const    pattern
    );

    /** Get the length of the string
      * @param src The string whose length is to be determined
      * @return Returns the length of the string
      */
    static XMLSize_t stringLen(const char* const src);

    /** Get the length of the string
      * @param src The string whose length is to be determined
      * @return Returns the length of the string
      */
    static XMLSize_t stringLen(const XMLCh* const src);

    /**
      *
      * Checks whether an name is a valid NOTATION according to XML 1.0
      * @param name    The string to check its NOTATION validity
      * @param manager The memory manager
      * @return Returns true if name is NOTATION valid, otherwise false
      */
    static bool isValidNOTATION(const XMLCh*         const name
                              ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
      * Checks whether an name is a valid EncName.
      * @param name The string to check its EncName validity
      * @return Returns true if name is EncName valid, otherwise false
      */
    static bool isValidEncName(const XMLCh* const name);

    /**
      * Checks whether a character is within [a-zA-Z].
      * @param theChar the character to check
      * @return Returns true if within the range, otherwise false
      */

    static bool isAlpha(XMLCh const theChar);

    /**
      * Checks whether a character is within [0-9].
      * @param theChar the character to check
      * @return Returns true if within the range, otherwise false
      */
    static bool isDigit(XMLCh const theChar);

    /**
      * Checks whether a character is within [0-9a-zA-Z].
      * @param theChar the character to check
      * @return Returns true if within the range, otherwise false
      */
    static bool isAlphaNum(XMLCh const theChar);

    /**
      * Checks whether a character is within [0-9a-fA-F].
      * @param theChar the character to check
      * @return Returns true if within the range, otherwise false
      */
    static bool isHex(XMLCh const theChar);

    /** Find is the string appears in the enum list
      * @param toFind the string to be found
      * @param enumList the list
      * return true if found
      */
    static bool isInList(const XMLCh* const toFind, const XMLCh* const enumList);

    //@}

    /** @name Conversion functions */
    //@{

      /** Converts size to a text string based a given radix
      *
      * @param toFormat The size to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void sizeToText
    (
        const   XMLSize_t           toFormat
        ,       char* const         toFill
        , const XMLSize_t           maxChars
        , const unsigned int        radix
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts size to a text string based a given radix
      *
      * @param toFormat The size to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void sizeToText
    (
        const   XMLSize_t           toFormat
        ,       XMLCh* const        toFill
        , const XMLSize_t           maxChars
        , const unsigned int        radix
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   unsigned int    toFormat
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   unsigned int    toFormat
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   unsigned long   toFormat
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   unsigned long   toFormat
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   int             toFormat
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   int             toFormat
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   long            toFormat
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts binary data to a text string based a given radix
      *
      * @param toFormat The number to convert
      * @param toFill The buffer that will hold the output on return. The
      *        size of this buffer should at least be 'maxChars + 1'.
      * @param maxChars The maximum number of output characters that can be
      *         accepted. If the result will not fit, it is an error.
      * @param radix The radix of the input data, based on which the conversion
      * @param manager The MemoryManager to use to allocate objects
      * will be done
      */
    static void binToText
    (
        const   long            toFormat
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , const unsigned int    radix
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Converts a string of decimal chars to a binary value
      *
      * Note that leading and trailing whitespace is legal and will be ignored
      * but the remainder must be all decimal digits.
      *
      * @param toConvert The string of digits to convert
      * @param toFill    The unsigned int value to fill with the converted
      *                  value.
      * @param manager The MemoryManager to use to allocate objects
      */
    static bool textToBin
    (
        const   XMLCh* const    toConvert
        ,       unsigned int&   toFill
        ,       MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Converts a string of decimal chars to a binary value
      *
      * Note that leading and trailing whitespace is legal and will be ignored,
      *
      * Only one and either of (+,-) after the leading whitespace, before
      * any other characters are allowed.
      *
      * but the remainder must be all decimal digits.
      *
      * @param toConvert The string of digits to convert
      * @param manager The MemoryManager to use to allocate objects
      */
    static int parseInt
    (
        const   XMLCh* const    toConvert
      , MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Cut leading chars from a string
      *
      * @param toCutFrom The string to cut chars from
      * @param count     The count of leading chars to cut
      */
    static void cut
    (
                XMLCh* const    toCutFrom
        , const XMLSize_t       count
    );

    /** Transcodes a string to native code-page
      *
      * NOTE: The returned buffer is dynamically allocated and is the
      * responsibility of the caller to delete it when not longer needed.
      * You can call XMLString::release to release this returned buffer.
      *
      * @param toTranscode The string to be transcoded
      * @param manager The MemoryManager to use to allocate objects
      * @return Returns the transcoded string
      * @see   XMLString::release(XMLCh**, MemoryManager*)
      */
    static char* transcode
    (
        const   XMLCh* const         toTranscode
        ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Transcodes a string to native code-page (DEPRECATED)
      *
      * Be aware that when transcoding to an external encoding, that each
      * Unicode char can create multiple output bytes. So you cannot assume
      * a one to one correspondence of input chars to output bytes.
      *
      * @param toTranscode The string tobe transcoded
      * @param toFill The buffer that is filled with the transcoded value.
      *        The size of this buffer should atleast be 'maxChars + 1'.
      * @param maxChars The maximum number of bytes that the output
      *         buffer can hold (not including the null, which is why
      *         toFill should be at least maxChars+1.).
      * @param manager The MemoryManager to use to allocate objects
      * @return Returns true if successful, false if there was an error
      */
    static bool transcode
    (
        const   XMLCh* const    toTranscode
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Transcodes a string to native code-page
      *
      * NOTE: The returned buffer is dynamically allocated and is the
      * responsibility of the caller to delete it when not longer needed.
      * You can call XMLString::release to release this returned buffer.
      *
      * @param toTranscode The string to be transcoded
      * @param manager The MemoryManager to use to allocate objects
      * @return Returns the transcoded string
      * @see   XMLString::release(char**, MemoryManager*)
      */
    static XMLCh* transcode
    (
        const   char* const          toTranscode
        ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Transcodes a string to native code-page (DEPRECATED)
      * @param toTranscode The string tobe transcoded
      * @param toFill The buffer that is filled with the transcoded value.
      *        The size of this buffer should atleast be 'maxChars + 1'.
      * @param maxChars The maximum number of characters that the output
      *         buffer can hold (not including the null, which is why
      *         toFill should be at least maxChars+1.).
      * @param manager The MemoryManager to use to allocate objects
      * @return Returns true if successful, false if there was an error
      */
    static bool transcode
    (
        const   char* const     toTranscode
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Trims off extra space characters from the start and end of the string,
      * moving the non-space string content back to the start.
      * @param toTrim The string to be trimmed. On return this contains the
      * trimmed string
      */
    static void trim(char* const toTrim);

    /** Trims off extra space characters from the start and end of the string,
      * moving the non-space string content back to the start.
      * @param toTrim The string to be trimmed. On return this contains
      * the trimmed string
      */
    static void trim(XMLCh* const toTrim);

    /** Break a string into tokens with space as delimiter, and
      * stored in a string vector.  The caller owns the string vector
      * that is returned, and is responsible for deleting it.
      * @param tokenizeSrc String to be tokenized
      * @param manager The MemoryManager to use to allocate objects
      * @return a vector of all the tokenized string
      */
    static BaseRefVectorOf<XMLCh>* tokenizeString(const XMLCh* const tokenizeSrc
                                        , MemoryManager*       const manager = XMLPlatformUtils::fgMemoryManager);

    //@}

    /** @name Formatting functions */
    //@{
    /** Creates a UName from a URI and base name. It is in the form
      * {url}name, and is commonly used internally to represent fully
      * qualified names when namespaces are enabled.
      *
      * @param pszURI The URI part of the name
      * @param pszName The base part of the name
      * @return Returns the complete formatted UName
      */
    static XMLCh* makeUName
    (
        const   XMLCh* const    pszURI
        , const XMLCh* const    pszName
    );

    /**
      * Internal function to perform token replacement for strings.
      *
      * @param errText The text (NULL terminated) where the replacement
      *        is to be done. The size of this buffer should be
      *        'maxChars + 1' to account for the final NULL.
      * @param maxChars The size of the output buffer, i.e. the maximum
      *         number of characters that it will hold. If the result is
      *         larger, it will be truncated.
      * @param text1 Replacement text-one
      * @param text2 Replacement text-two
      * @param text3 Replacement text-three
      * @param text4 Replacement text-four
      * @param manager The MemoryManager to use to allocate objects
      * @return Returns the count of characters that are outputted
      */
    static XMLSize_t replaceTokens
    (
                XMLCh* const    errText
        , const XMLSize_t       maxChars
        , const XMLCh* const    text1
        , const XMLCh* const    text2
        , const XMLCh* const    text3
        , const XMLCh* const    text4
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Converts a string to uppercase
      * @param toUpperCase The string which needs to be converted to uppercase.
      *        On return, this buffer also holds the converted uppercase string
      */
    static void upperCase(XMLCh* const toUpperCase);

    /** Converts a string to uppercase
      * The routine only uppercases A to Z (other characters not changed).
      * @param toUpperCase The string which needs to be converted to uppercase.
      *        On return, this buffer also holds the converted uppercase string
      */
    static void upperCaseASCII(XMLCh* const toUpperCase);

	/** Converts a string to lowercase
      * @param toLowerCase The string which needs to be converted to lowercase.
      *        On return, this buffer also holds the converted lowercase string
      */
    static void lowerCase(XMLCh* const toLowerCase);

    /** Converts a string to lowercase
      * The routine only lowercases a to z (other characters not changed).
      * @param toLowerCase The string which needs to be converted to lowercase.
      *        On return, this buffer also holds the converted lowercase string
      */
    static void lowerCaseASCII(XMLCh* const toLowerCase);

	/** Check if string is WhiteSpace:replace
      * @param toCheck The string which needs to be checked.
      */
    static bool isWSReplaced(const XMLCh* const toCheck);

	/** Check if string is WhiteSpace:collapse
      * @param toCheck The string which needs to be checked.
      */
    static bool isWSCollapsed(const XMLCh* const toCheck);

	/** Replace whitespace
      * @param toConvert The string which needs to be whitespace replaced.
      *        On return , this buffer also holds the converted string
      * @param manager The MemoryManager to use to allocate objects
      */
    static void replaceWS(XMLCh* toConvert
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager);

	/** Collapse whitespace
      * @param toConvert The string which needs to be whitespace collapsed.
      *        On return , this buffer also holds the converted string
      * @param manager The MemoryManager to use to allocate objects
      */
    static void collapseWS(XMLCh* toConvert
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager);

    /** Remove whitespace
      * @param toConvert The string which needs to be whitespace removed.
      *        On return , this buffer also holds the converted string
      * @param manager The MemoryManager to use to allocate objects
      */
    static void removeWS(XMLCh* toConvert
    , MemoryManager*       const manager = XMLPlatformUtils::fgMemoryManager);


    /** Remove character
      * @param srcString The string
      * @param toRemove  The character needs to be removed from the string
      * @param dstBuffer The buffer containing the result
      */
    static void removeChar(const XMLCh*     const srcString
                         , const XMLCh&           toRemove
                         ,       XMLBuffer&       dstBuffer);

    /**
     * Fixes a platform dependent absolute path filename to standard URI form.
     * 1. Windows: fix 'x:' to 'file:///x:' and convert any backslash to forward slash
     * 2. UNIX: fix '/blah/blahblah' to 'file:///blah/blahblah'
     * @param str    The string that has the absolute path filename
     * @param target The target string pre-allocated to store the fixed uri
     */
    static void fixURI(const XMLCh* const str, XMLCh* const target);

    //@}
    /** @name String Memory Management functions */
    //@{
    /**
     * Release the parameter string that was allocated by XMLString::transcode and XMLString::replicate.
     * The implementation will call MemoryManager::deallocate and then turn the string to a null pointer.
     *
     * @param buf  The string to be deleted and become a null pointer.
     * @param manager The MemoryManager used to allocate the string
     */
    static void release
    (
        char**  buf
        ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
     * Release the parameter string that was allocated by XMLString::transcode and XMLString::replicate.
     * The implementation will call MemoryManager::deallocate and then turn the string to a null pointer.
     *
     * @param buf  The string to be deleted and become a null pointer.
     * @param manager The MemoryManager used to allocate the string
     */
    static void release
    (
        XMLCh**  buf
        ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    //@}


private :

    /** @name Constructors and Destructor */
    //@{
    /** Unimplemented default constructor */
    XMLString();
    /** Unimplemented destructor */
    ~XMLString();
    //@}


    /** @name Initialization */
    //@{
    /** Init/Term methods called from XMLPlatformUtils class */
    static void initString(XMLLCPTranscoder* const defToUse,
                           MemoryManager* const manager);
    static void termString();
    //@}

	/**
	  * Called by regionMatches/regionIMatches to validate that we
	  * have a valid input
	  */
	static bool validateRegion(const XMLCh* const str1, const int offset1,
						const XMLCh* const str2, const int offset2,
						const XMLSize_t charCount);

    static MemoryManager* fgMemoryManager;

    friend class XMLPlatformUtils;
};


// ---------------------------------------------------------------------------
//  Inline some methods that are either just passthroughs to other string
//  methods, or which are key for performance.
// ---------------------------------------------------------------------------
inline void XMLString::moveChars(       XMLCh* const targetStr
                                , const XMLCh* const srcStr
                                , const XMLSize_t    count)
{
    memcpy(targetStr, srcStr, count * sizeof(XMLCh));
}

inline XMLSize_t XMLString::stringLen(const XMLCh* const src)
{
    if (src == 0)
        return 0;

    const XMLCh* pszTmp = src;

    while (*pszTmp++) ;

    return (pszTmp - src - 1);
}

inline XMLCh* XMLString::replicate(const XMLCh* const toRep,
                                   MemoryManager* const manager)
{
    // If a null string, return a null string!
    XMLCh* ret = 0;
    if (toRep)
    {
        const XMLSize_t len = stringLen(toRep);
        ret = (XMLCh*) manager->allocate((len+1) * sizeof(XMLCh)); //new XMLCh[len + 1];
        memcpy(ret, toRep, (len + 1) * sizeof(XMLCh));
    }
    return ret;
}

inline bool XMLString::startsWith(  const   XMLCh* const    toTest
                                    , const XMLCh* const    prefix)
{
    return (compareNString(toTest, prefix, stringLen(prefix)) == 0);
}

inline bool XMLString::startsWithI( const   XMLCh* const    toTest
                                    , const XMLCh* const    prefix)
{
    return (compareNIString(toTest, prefix, stringLen(prefix)) == 0);
}

inline bool XMLString::endsWith(const XMLCh* const toTest,
                                const XMLCh* const suffix)
{

    XMLSize_t suffixLen = XMLString::stringLen(suffix);

    return regionMatches(toTest, (int)(XMLString::stringLen(toTest) - suffixLen),
                         suffix, 0, suffixLen);
}

inline bool XMLString::validateRegion(const XMLCh* const str1,
									  const int offset1,
									  const XMLCh* const str2,
									  const int offset2,
									  const XMLSize_t charCount)
{

	if (offset1 < 0 || offset2 < 0 ||
		(offset1 + charCount) > XMLString::stringLen(str1) ||
		(offset2 + charCount) > XMLString::stringLen(str2) )
		return false;

	return true;
}

inline bool XMLString::equals(   const XMLCh* str1
                               , const XMLCh* str2)
{
    if (str1 == str2)
        return true;

    if (str1 == 0 || str2 == 0)
        return ((!str1 || !*str1) && (!str2 || !*str2));

    while (*str1)
        if(*str1++ != *str2++)  // they are different (or str2 is shorter and we hit the NULL)
            return false;

    // either both ended (and *str2 is 0 too), or str2 is longer
    return (*str2==0);
}

inline bool XMLString::equalsN(const XMLCh* str1,
                               const XMLCh* str2,
                               XMLSize_t n)
{
    if (str1 == str2 || n == 0)
      return true;

    if (str1 == 0 || str2 == 0)
        return ((!str1 || !*str1) && (!str2 || !*str2));

    for (; n != 0 && *str1 && *str2; --n, ++str1, ++str2)
      if(*str1 != *str2)
        break;

    return n == 0 || *str1 == *str2; // either equal or both ended premat.
}

inline bool XMLString::equals(   const char* str1
                               , const char* str2)
{
    if (str1 == str2)
        return true;

    if (str1 == 0 || str2 == 0)
        return ((!str1 || !*str1) && (!str2 || !*str2));

    while (*str1)
        if(*str1++ != *str2++)  // they are different (or str2 is shorter and we hit the NULL)
            return false;

    // either both ended (and *str2 is 0 too), or str2 is longer
    return (*str2==0);
}

inline bool XMLString::equalsN(const char* str1,
                               const char* str2,
                               XMLSize_t n)
{
    if (str1 == str2 || n == 0)
      return true;

    if (str1 == 0 || str2 == 0)
        return ((!str1 || !*str1) && (!str2 || !*str2));

    for (; n != 0 && *str1 && *str2; --n, ++str1, ++str2)
      if(*str1 != *str2)
        break;

    return n == 0 || *str1 == *str2; // either equal or both ended premat.
}

inline int XMLString::lastIndexOf(const XMLCh* const toSearch, const XMLCh ch)
{
    return XMLString::lastIndexOf(ch, toSearch, stringLen(toSearch));
}

inline XMLSize_t XMLString::hash(const   XMLCh* const   tohash
                                , const XMLSize_t          hashModulus)
{
    if (tohash == 0 || *tohash == 0)
        return 0;

    const XMLCh* curCh = tohash;
    XMLSize_t hashVal = (XMLSize_t)(*curCh++);

    while (*curCh)
        hashVal = (hashVal * 38) + (hashVal >> 24) + (XMLSize_t)(*curCh++);

    // Divide by modulus
    return hashVal % hashModulus;
}

inline XMLSize_t XMLString::hashN(const   XMLCh* const   tohash
                                  , const XMLSize_t       n
                                  , const XMLSize_t       hashModulus)
{
  if (tohash == 0 || n == 0)
    return 0;

  const XMLCh* curCh = tohash;
  XMLSize_t hashVal = (XMLSize_t)(*curCh++);

  for(XMLSize_t i=0;i<n;i++)
    hashVal = (hashVal * 38) + (hashVal >> 24) + (XMLSize_t)(*curCh++);

  // Divide by modulus
  return hashVal % hashModulus;
}

XERCES_CPP_NAMESPACE_END

#endif
