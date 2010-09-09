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
 * $Id: XMLFormatter.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLFORMATTER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLFORMATTER_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLFormatTarget;
class XMLTranscoder;

/**
 *  This class provides the basic formatting capabilities that are required
 *  to turn the Unicode based XML data from the parsers into a form that can
 *  be used on non-Unicode based systems, that is, into local or generic text
 *  encodings.
 *
 *  A number of flags are provided to control whether various optional
 *  formatting operations are performed.
 */
class XMLPARSER_EXPORT XMLFormatter : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Class types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * EscapeFlags - Different styles of escape flags to control various formatting.
     *
     * <p><code>NoEscapes:</code>
     * No character needs to be escaped.   Just write them out as is.</p>
     * <p><code>StdEscapes:</code>
     * The following characters need to be escaped:</p>
     * <table border='1'>
     * <tr>
     * <td>character</td>
     * <td>should be escaped and written as</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&amp;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;amp;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&gt;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;gt;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&quot;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;quot;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&lt;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;lt;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&apos;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;apos;</td>
     * </tr>
     * </table>
     * <p><code>AttrEscapes:</code>
     * The following characters need to be escaped:</p>
     * <table border='1'>
     * <tr>
     * <td>character</td>
     * <td>should be escaped and written as</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&amp;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;amp;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&gt;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;gt;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&quot;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;quot;</td>
     * </tr>
     * </table>
     * <p><code>CharEscapes:</code>
     * The following characters need to be escaped:</p>
     * <table border='1'>
     * <tr>
     * <td>character</td>
     * <td>should be escaped and written as</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&amp;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;amp;</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>&gt;</td>
     * <td valign='top' rowspan='1' colspan='1'>&amp;gt;</td>
     * </tr>
     * </table>
     * <p><code>EscapeFlags_Count:</code>
     * Special value, do not use directly.</p>
     * <p><code>DefaultEscape:</code>
     * Special value, do not use directly.</p>
     *
     */
    enum EscapeFlags
    {
        NoEscapes
        , StdEscapes
        , AttrEscapes
        , CharEscapes

        // Special values, don't use directly
        , EscapeFlags_Count
        , DefaultEscape     = 999
    };

    /**
     * UnRepFlags
     *
     * The unrepresentable flags that indicate how to react when a
     * character cannot be represented in the target encoding.
     *
     * <p><code>UnRep_Fail:</code>
     * Fail the operation.</p>
     * <p><code>UnRep_CharRef:</code>
     * Display the unrepresented character as reference.</p>
     * <p><code>UnRep_Replace:</code>
     * Replace the unrepresented character with the replacement character.</p>
     * <p><code>DefaultUnRep:</code>
     * Special value, do not use directly.</p>
     *
     */
    enum UnRepFlags
    {
        UnRep_Fail
        , UnRep_CharRef
        , UnRep_Replace

        , DefaultUnRep      = 999
    };
    //@}


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructor and Destructor */
    //@{
    /**
     * @param outEncoding the encoding for the formatted content.
     * @param docVersion  the document version.
     * @param target      the formatTarget where the formatted content is written to.
     * @param escapeFlags the escape style for certain character.
     * @param unrepFlags  the reaction to unrepresentable character.
     * @param manager     Pointer to the memory manager to be used to
     *                    allocate objects.
     */
    XMLFormatter
    (
        const   XMLCh* const            outEncoding
        , const XMLCh* const            docVersion
        ,       XMLFormatTarget* const  target
        , const EscapeFlags             escapeFlags = NoEscapes
        , const UnRepFlags              unrepFlags = UnRep_Fail
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    XMLFormatter
    (
        const   char* const             outEncoding
        , const char* const             docVersion
        ,       XMLFormatTarget* const  target
        , const EscapeFlags             escapeFlags = NoEscapes
        , const UnRepFlags              unrepFlags = UnRep_Fail
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    XMLFormatter
    (
        const   XMLCh* const            outEncoding
        ,       XMLFormatTarget* const  target
        , const EscapeFlags             escapeFlags = NoEscapes
        , const UnRepFlags              unrepFlags = UnRep_Fail
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    XMLFormatter
    (
        const   char* const             outEncoding
        ,       XMLFormatTarget* const  target
        , const EscapeFlags             escapeFlags = NoEscapes
        , const UnRepFlags              unrepFlags = UnRep_Fail
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    ~XMLFormatter();
    //@}


    // -----------------------------------------------------------------------
    //  Formatting methods
    // -----------------------------------------------------------------------
    /** @name Formatting methods */
    //@{
    /**
     * @param toFormat the string to be formatted
     * @param count    length of the string
     * @param escapeFlags the escape style for formatting toFormat
     * @param unrepFlags the reaction for any unrepresentable character in toFormat
     *
     */
    void formatBuf
    (
        const   XMLCh* const    toFormat
        , const XMLSize_t       count
        , const EscapeFlags     escapeFlags = DefaultEscape
        , const UnRepFlags      unrepFlags = DefaultUnRep
    );

    /**
     * @see formatBuf
     */
    XMLFormatter& operator<<
    (
        const   XMLCh* const    toFormat
    );

    XMLFormatter& operator<<
    (
        const   XMLCh           toFormat
    );

    void writeBOM(const XMLByte* const toFormat
                , const XMLSize_t      count);

    //@}

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /** @name Getter methods */
    //@{
    /**
     * @return return the encoding set for the formatted content
     */

    const XMLCh* getEncodingName() const;

    /**
     * @return return constant transcoder used internally for transcoding the formatter conent
     */
    inline const XMLTranscoder*   getTranscoder() const;

    /**
     * @return return the transcoder used internally for transcoding the formatter content
     */
    inline XMLTranscoder*   getTranscoder();

   //@}

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /** @name Setter methods */
    //@{
    /**
     * @param newFlags set the escape style for the follow-on formatted content
     */
    void setEscapeFlags
    (
        const   EscapeFlags     newFlags
    );

    /**
     * @param newFlags set the reaction for unrepresentable character
     */
    void setUnRepFlags
    (
        const   UnRepFlags      newFlags
    );

    /**
     * @param newFlags set the escape style for the follow-on formatted content
     * @see setEscapeFlags
     */
    XMLFormatter& operator<<
    (
        const   EscapeFlags     newFlags
    );

    /**
     * @param newFlags set the reaction for unrepresentable character
     * @see setUnRepFlags
     */
    XMLFormatter& operator<<
    (
        const   UnRepFlags      newFlags
    );
    //@}

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /** @name Setter methods */
    //@{
    /**
     * @return return the escape style for the formatted content
     */
    EscapeFlags getEscapeFlags() const;

    /**
     * @return return the reaction for unrepresentable character
     */
    UnRepFlags getUnRepFlags() const;
    //@}

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLFormatter();
    XMLFormatter(const XMLFormatter&);
    XMLFormatter& operator=(const XMLFormatter&);


    // -----------------------------------------------------------------------
    //  Private class constants
    // -----------------------------------------------------------------------
    enum Constants
    {
        kTmpBufSize     = 16 * 1024
    };


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    const XMLByte* getCharRef(XMLSize_t     &count,
                              XMLByte*      &ref,
                              const XMLCh *  stdRef);

    void writeCharRef(const XMLCh &toWrite);
    void writeCharRef(XMLSize_t toWrite);

    bool inEscapeList(const XMLFormatter::EscapeFlags escStyle
                    , const XMLCh                     toCheck);


    XMLSize_t handleUnEscapedChars(const XMLCh *      srcPtr,
                                   const XMLSize_t    count,
                                   const UnRepFlags   unrepFlags);

    void specialFormat
    (
        const   XMLCh* const    toFormat
        , const XMLSize_t       count
        , const EscapeFlags     escapeFlags
    );


    // -----------------------------------------------------------------------
    //  Private, non-virtual methods
    //
    //  fEscapeFlags
    //      The escape flags we were told to use in formatting. These are
    //      defaults set in the ctor, which can be overridden on a particular
    //      call.
    //
    //  fOutEncoding
    //      This the name of the output encoding. Saved mainly for meaningful
    //      error messages.
    //
    //  fTarget
    //      This is the target object for the formatting operation.
    //
    //  fUnRepFlags
    //      The unrepresentable flags that indicate how to react when a
    //      character cannot be represented in the target encoding.
    //
    //  fXCoder
    //      This the transcoder that we will use. It is created using the
    //      encoding name we were told to use.
    //
    //  fTmpBuf
    //      An output buffer that we use to transcode chars into before we
    //      send them off to be output.
    //
    //  fAposRef
    //  fAmpRef
    //  fGTRef
    //  fLTRef
    //  fQuoteRef
    //      These are character refs for the standard char refs, in the
    //      output encoding. They are faulted in as required, by transcoding
    //      them from fixed Unicode versions.
    //
    //  fIsXML11
    //      for performance reason, we do not store the actual version string
    //      and do the string comparison again and again.
    //
    // -----------------------------------------------------------------------
    EscapeFlags                 fEscapeFlags;
    XMLCh*                      fOutEncoding;
    XMLFormatTarget*            fTarget;
    UnRepFlags                  fUnRepFlags;
    XMLTranscoder*              fXCoder;
    XMLByte                     fTmpBuf[kTmpBufSize + 4];
    XMLByte*                    fAposRef;
    XMLSize_t                   fAposLen;
    XMLByte*                    fAmpRef;
    XMLSize_t                   fAmpLen;
    XMLByte*                    fGTRef;
    XMLSize_t                   fGTLen;
    XMLByte*                    fLTRef;
    XMLSize_t                   fLTLen;
    XMLByte*                    fQuoteRef;
    XMLSize_t                   fQuoteLen;
    bool                        fIsXML11;
    MemoryManager*              fMemoryManager;
};


class XMLPARSER_EXPORT XMLFormatTarget : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    virtual ~XMLFormatTarget() {}


    // -----------------------------------------------------------------------
    //  Virtual interface
    // -----------------------------------------------------------------------
    virtual void writeChars
    (
          const XMLByte* const      toWrite
        , const XMLSize_t           count
        ,       XMLFormatter* const formatter
    ) = 0;

    virtual void flush() {};


protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors and operators
    // -----------------------------------------------------------------------
    XMLFormatTarget() {};

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLFormatTarget(const XMLFormatTarget&);
    XMLFormatTarget& operator=(const XMLFormatTarget&);
};


// ---------------------------------------------------------------------------
//  XMLFormatter: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* XMLFormatter::getEncodingName() const
{
    return fOutEncoding;
}

inline const XMLTranscoder* XMLFormatter::getTranscoder() const
{
    return fXCoder;
}

inline XMLTranscoder* XMLFormatter::getTranscoder()
{
    return fXCoder;
}

// ---------------------------------------------------------------------------
//  XMLFormatter: Setter methods
// ---------------------------------------------------------------------------
inline void XMLFormatter::setEscapeFlags(const EscapeFlags newFlags)
{
    fEscapeFlags = newFlags;
}

inline void XMLFormatter::setUnRepFlags(const UnRepFlags newFlags)
{
    fUnRepFlags = newFlags;
}


inline XMLFormatter& XMLFormatter::operator<<(const EscapeFlags newFlags)
{
    fEscapeFlags = newFlags;
    return *this;
}

inline XMLFormatter& XMLFormatter::operator<<(const UnRepFlags newFlags)
{
    fUnRepFlags = newFlags;
    return *this;
}

// ---------------------------------------------------------------------------
//  XMLFormatter: Getter methods
// ---------------------------------------------------------------------------
inline XMLFormatter::EscapeFlags XMLFormatter::getEscapeFlags() const
{
    return fEscapeFlags;
}

inline XMLFormatter::UnRepFlags XMLFormatter::getUnRepFlags() const
{
    return fUnRepFlags;
}

XERCES_CPP_NAMESPACE_END

#endif
