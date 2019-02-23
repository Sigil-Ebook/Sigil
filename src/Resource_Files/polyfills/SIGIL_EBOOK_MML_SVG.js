/*************************************************************
 *
 *  Modified Version of MathJax/unpacked/config/MML_SVG.js
 *  Specifically for Sigil-EBook's PreviewWindow
 *
 *  Copyright (c) 2010-2018 The MathJax Consortium
 *
 *  Part of the MathJax library.
 *  See http://www.mathjax.org for details.
 * 
 *  Licensed under the Apache License, Version 2.0;
 *  you may not use this file except in compliance with the License.
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 */

MathJax.Hub.Config({
  extensions: ["mml2jax.js","MathML/mml3.js","MathEvents.js","fast-preview.js","AssistiveMML.js"],
  jax: ["input/MathML","output/SVG","output/PreviewHTML"],
  messageStyle: "none"
});

MathJax.Ajax.loadComplete("[MathJax]/config/local/SIGIL_EBOOK_MML_SVG.js");
