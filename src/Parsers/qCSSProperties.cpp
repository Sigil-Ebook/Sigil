/************************************************************************
 **
 **  Copyright (C) 2021  Kevin B. Hendricks, Stratford, Ontario, Canada
 **
 **  This file is part of Sigil.
 **
 **  Sigil is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Sigil is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
 **
 ** Extracted and modified from:
 ** CSSTidy (https://github.com/csstidy-c/csstidy)
 **
 ** CSSTidy Portions Copyright:
 **   Florian Schmitz <floele@gmail.com>
 **   Thierry Charbonnel
 **   Will Mitchell <aethon@gmail.com>
 **   Brett Zamir <brettz9@yahoo.com>
 **   sined_ <sined_@users.sourceforge.net>
 **   Dmitry Leskov <git@dmitryleskov.com>
 **   Kevin Coyner <kcoyner@debian.org>
 **   Tuukka Pasanen <pasanen.tuukka@gmail.com>
 **   Frank W. Bergmann <csstidy-c@tuxad.com>
 **   Frank Dana <ferdnyc@gmail.com>
 **
 ** CSSTidy us Available under the LGPL 2.1
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include <QString>
#include "Parsers/qCSSProperties.h"

CSSProperties *CSSProperties::m_instance = 0;

CSSProperties *CSSProperties::instance()
{
    if (m_instance == 0) {
        m_instance = new CSSProperties();
    }
    return m_instance;
}


bool CSSProperties::contains(QString pname)
{
    return m_all_properties.contains(pname);
}


QString CSSProperties::levels(QString pname)
{
    if (contains(pname)) {
        return m_all_properties[pname];
    }
    return "";
}


CSSProperties::CSSProperties()
{
    if (m_all_properties.empty()) {
        m_all_properties["alignment-adjust"] = "CSS3.0";
        m_all_properties["alignment-baseline"] = "CSS3.0";
        m_all_properties["animation"] = "CSS3.0";
        m_all_properties["animation-delay"] = "CSS3.0";
        m_all_properties["animation-direction"] = "CSS3.0";
        m_all_properties["animation-duration"] = "CSS3.0";
        m_all_properties["animation-iteration-count"] = "CSS3.0";
        m_all_properties["animation-name"] = "CSS3.0";
        m_all_properties["animation-play-state"] = "CSS3.0";
        m_all_properties["animation-timing-function"] = "CSS3.0";
        m_all_properties["appearance"] = "CSS3.0";
        m_all_properties["azimuth"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["backface-visibility"] = "CSS3.0";
        m_all_properties["background"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-attachment"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-clip"] = "CSS3.0";
        m_all_properties["background-color"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-image"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-origin"] = "CSS3.0";
        m_all_properties["background-position"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-repeat"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["background-size"] = "CSS3.0";
        m_all_properties["baseline-shift"] = "CSS3.0";
        m_all_properties["binding"] = "CSS3.0";
        m_all_properties["bleed"] = "CSS3.0";
        m_all_properties["bookmark-label"] = "CSS3.0";
        m_all_properties["bookmark-level"] = "CSS3.0";
        m_all_properties["bookmark-state"] = "CSS3.0";
        m_all_properties["bookmark-target"] = "CSS3.0";
        m_all_properties["border"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-bottom"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-bottom-color"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-bottom-left-radius"] = "CSS3.0";
        m_all_properties["border-bottom-right-radius"] = "CSS3.0";
        m_all_properties["border-bottom-style"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-bottom-width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-collapse"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-color"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-image"] = "CSS3.0";
        m_all_properties["border-image-outset"] = "CSS3.0";
        m_all_properties["border-image-repeat"] = "CSS3.0";
        m_all_properties["border-image-slice"] = "CSS3.0";
        m_all_properties["border-image-source"] = "CSS3.0";
        m_all_properties["border-image-width"] = "CSS3.0";
        m_all_properties["border-left"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-left-color"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-left-style"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-left-width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-radius"] = "CSS3.0";
        m_all_properties["border-right"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-right-color"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-right-style"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-right-width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-spacing"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-style"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-top"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-top-color"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-top-left-radius"] = "CSS3.0";
        m_all_properties["border-top-right-radius"] = "CSS3.0";
        m_all_properties["border-top-style"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-top-width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["border-width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["bottom"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["box-decoration-break"] = "CSS3.0";
        m_all_properties["box-shadow"] = "CSS3.0";
        m_all_properties["box-sizing"] = "CSS3.0";
        m_all_properties["break-after"] = "CSS3.0";
        m_all_properties["break-before"] = "CSS3.0";
        m_all_properties["break-inside"] = "CSS3.0";
        m_all_properties["caption-side"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["clear"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["clip"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["color"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["color-profile"] = "CSS3.0";
        m_all_properties["column-count"] = "CSS3.0";
        m_all_properties["column-fill"] = "CSS3.0";
        m_all_properties["column-gap"] = "CSS3.0";
        m_all_properties["column-rule"] = "CSS3.0";
        m_all_properties["column-rule-color"] = "CSS3.0";
        m_all_properties["column-rule-style"] = "CSS3.0";
        m_all_properties["column-rule-width"] = "CSS3.0";
        m_all_properties["column-span"] = "CSS3.0";
        m_all_properties["column-width"] = "CSS3.0";
        m_all_properties["columns"] = "CSS3.0";
        m_all_properties["content"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["counter-increment"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["counter-reset"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["crop"] = "CSS3.0";
        m_all_properties["cue"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["cue-after"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["cue-before"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["cursor"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["direction"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["display"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["dominant-baseline"] = "CSS3.0";
        m_all_properties["drop-initial-after-adjust"] = "CSS3.0";
        m_all_properties["drop-initial-after-align"] = "CSS3.0";
        m_all_properties["drop-initial-before-adjust"] = "CSS3.0";
        m_all_properties["drop-initial-before-align"] = "CSS3.0";
        m_all_properties["drop-initial-size"] = "CSS3.0";
        m_all_properties["drop-initial-value"] = "CSS3.0";
        m_all_properties["elevation"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["empty-cells"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["fit"] = "CSS3.0";
        m_all_properties["fit-position"] = "CSS3.0";
        m_all_properties["flex-align"] = "CSS3.0";
        m_all_properties["flex-flow"] = "CSS3.0";
        m_all_properties["flex-line-pack"] = "CSS3.0";
        m_all_properties["flex-order"] = "CSS3.0";
        m_all_properties["flex-pack"] = "CSS3.0";
        m_all_properties["float"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["float-offset"] = "CSS3.0";
        m_all_properties["font"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["font-family"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["font-format"] = "CSS3.0";
        m_all_properties["font-size"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["font-size-adjust"] = "CSS2.0,CSS3.0";
        m_all_properties["font-stretch"] = "CSS2.0,CSS3.0";
        m_all_properties["font-style"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["font-variant"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["font-weight"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["grid-columns"] = "CSS3.0";
        m_all_properties["grid-rows"] = "CSS3.0";
        m_all_properties["hanging-punctuation"] = "CSS3.0";
        m_all_properties["height"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["hyphenate-after"] = "CSS3.0";
        m_all_properties["hyphenate-before"] = "CSS3.0";
        m_all_properties["hyphenate-character"] = "CSS3.0";
        m_all_properties["hyphenate-lines"] = "CSS3.0";
        m_all_properties["hyphenate-resource"] = "CSS3.0";
        m_all_properties["hyphens"] = "CSS3.0";
        m_all_properties["icon"] = "CSS3.0";
        m_all_properties["image-orientation"] = "CSS3.0";
        m_all_properties["image-rendering"] = "CSS3.0";
        m_all_properties["image-resolution"] = "CSS3.0";
        m_all_properties["inline-box-align"] = "CSS3.0";
        m_all_properties["left"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["letter-spacing"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["line-break"] = "CSS3.0";
        m_all_properties["line-height"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["line-stacking"] = "CSS3.0";
        m_all_properties["line-stacking-ruby"] = "CSS3.0";
        m_all_properties["line-stacking-shift"] = "CSS3.0";
        m_all_properties["line-stacking-strategy"] = "CSS3.0";
        m_all_properties["list-style"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["list-style-image"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["list-style-position"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["list-style-type"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["margin"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["margin-bottom"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["margin-left"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["margin-right"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["margin-top"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["marker-offset"] = "CSS2.0,CSS3.0";
        m_all_properties["marks"] = "CSS2.0,CSS3.0";
        m_all_properties["marquee-direction"] = "CSS3.0";
        m_all_properties["marquee-loop"] = "CSS3.0";
        m_all_properties["marquee-play-count"] = "CSS3.0";
        m_all_properties["marquee-speed"] = "CSS3.0";
        m_all_properties["marquee-style"] = "CSS3.0";
        m_all_properties["max-height"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["max-width"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["min-height"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["min-width"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["move-to"] = "CSS3.0";
        m_all_properties["nav-down"] = "CSS3.0";
        m_all_properties["nav-index"] = "CSS3.0";
        m_all_properties["nav-left"] = "CSS3.0";
        m_all_properties["nav-right"] = "CSS3.0";
        m_all_properties["nav-up"] = "CSS3.0";
        m_all_properties["opacity"] = "CSS3.0";
        m_all_properties["orphans"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["outline"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["outline-color"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["outline-offset"] = "CSS3.0";
        m_all_properties["outline-style"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["outline-width"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["overflow"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["overflow-style"] = "CSS3.0";
        m_all_properties["overflow-wrap"] = "CSS3.0";
        m_all_properties["overflow-x"] = "CSS3.0";
        m_all_properties["overflow-y"] = "CSS3.0";
        m_all_properties["padding"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["padding-bottom"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["padding-left"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["padding-right"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["padding-top"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["page"] = "CSS2.0,CSS3.0";
        m_all_properties["page-break-after"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["page-break-before"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["page-break-inside"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["page-policy"] = "CSS3.0";
        m_all_properties["pause"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["pause-after"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["pause-before"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["perspective"] = "CSS3.0";
        m_all_properties["perspective-origin"] = "CSS3.0";
        m_all_properties["phonemes"] = "CSS3.0";
        m_all_properties["pitch"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["pitch-range"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["play-during"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["position"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["presentation-level"] = "CSS3.0";
        m_all_properties["punctuation-trim"] = "CSS3.0";
        m_all_properties["quotes"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["rendering-intent"] = "CSS3.0";
        m_all_properties["resize"] = "CSS3.0";
        m_all_properties["rest"] = "CSS3.0";
        m_all_properties["rest-after"] = "CSS3.0";
        m_all_properties["rest-before"] = "CSS3.0";
        m_all_properties["richness"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["right"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["rotation"] = "CSS3.0";
        m_all_properties["rotation-point"] = "CSS3.0";
        m_all_properties["ruby-align"] = "CSS3.0";
        m_all_properties["ruby-overhang"] = "CSS3.0";
        m_all_properties["ruby-position"] = "CSS3.0";
        m_all_properties["ruby-span"] = "CSS3.0";
        m_all_properties["size"] = "CSS2.0,CSS3.0";
        m_all_properties["speak"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["speak-header"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["speak-numeral"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["speak-punctuation"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["speech-rate"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["src"] = "CSS3.0";
        m_all_properties["stress"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["string-set"] = "CSS3.0";
        m_all_properties["tab-size"] = "CSS3.0";
        m_all_properties["table-layout"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["target"] = "CSS3.0";
        m_all_properties["target-name"] = "CSS3.0";
        m_all_properties["target-new"] = "CSS3.0";
        m_all_properties["target-position"] = "CSS3.0";
        m_all_properties["text-align"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["text-align-last"] = "CSS3.0";
        m_all_properties["text-decoration"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["text-decoration-color"] = "CSS3.0";
        m_all_properties["text-decoration-line"] = "CSS3.0";
        m_all_properties["text-decoration-skip"] = "CSS3.0";
        m_all_properties["text-decoration-style"] = "CSS3.0";
        m_all_properties["text-emphasis"] = "CSS3.0";
        m_all_properties["text-emphasis-color"] = "CSS3.0";
        m_all_properties["text-emphasis-position"] = "CSS3.0";
        m_all_properties["text-emphasis-style"] = "CSS3.0";
        m_all_properties["text-height"] = "CSS3.0";
        m_all_properties["text-indent"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["text-justify"] = "CSS3.0";
        m_all_properties["text-outline"] = "CSS3.0";
        m_all_properties["text-shadow"] = "CSS2.0,CSS3.0";
        m_all_properties["text-space-collapse"] = "CSS3.0";
        m_all_properties["text-transform"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["text-underline-position"] = "CSS3.0";
        m_all_properties["text-wrap"] = "CSS3.0";
        m_all_properties["top"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["transform"] = "CSS3.0";
        m_all_properties["transform-origin"] = "CSS3.0";
        m_all_properties["transform-style"] = "CSS3.0";
        m_all_properties["transition"] = "CSS3.0";
        m_all_properties["transition-delay"] = "CSS3.0";
        m_all_properties["transition-duration"] = "CSS3.0";
        m_all_properties["transition-property"] = "CSS3.0";
        m_all_properties["transition-timing-function"] = "CSS3.0";
        m_all_properties["unicode-bidi"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["vertical-align"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["visibility"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["voice-balance"] = "CSS3.0";
        m_all_properties["voice-duration"] = "CSS3.0";
        m_all_properties["voice-family"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["voice-pitch"] = "CSS3.0";
        m_all_properties["voice-pitch-range"] = "CSS3.0";
        m_all_properties["voice-rate"] = "CSS3.0";
        m_all_properties["voice-stress"] = "CSS3.0";
        m_all_properties["voice-volume"] = "CSS3.0";
        m_all_properties["volume"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["white-space"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["widows"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["width"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["word-break"] = "CSS3.0";
        m_all_properties["word-spacing"] = "CSS1.0,CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["word-wrap"] = "CSS3.0";
        m_all_properties["z-index"] = "CSS2.0,CSS2.1,CSS3.0";
        m_all_properties["zoom"] = "CSS3.0";
        m_all_properties["-epub-text-orientation"] = "CSS3.0";
        m_all_properties["-epub-writing-mode"] = "CSS3.0";
        m_all_properties["-epub-text-combine"] = "CSS3.0";
        m_all_properties["-epub-hyphens"] = "CSS3.0";
        m_all_properties["-epub-line-break"] = "CSS3.0";
        m_all_properties["-epub-text-align-last"] = "CSS3.0";
        m_all_properties["-epub-word-break"] = "CSS3.0";
        m_all_properties["-epub-text-emphasis-color"] = "CSS3.0";
        m_all_properties["-epub-text-emphasis-position"] = "CSS3.0";
        m_all_properties["-epub-text-emphasis-style"] = "CSS3.0";
        m_all_properties["-epub-text-underline-position"] = "CSS3.0";
    }
}
