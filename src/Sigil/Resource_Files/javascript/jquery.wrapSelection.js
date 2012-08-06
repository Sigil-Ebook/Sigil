/**
 * wrapSelection jQuery plugin v0.2 beta-2
 * @copyright	Copyright (c) 2008, Crossway Books
 * @author		Stephen Smith
 * @author		Jeremy Peterson
 * @version		0.2.0
 */
(function ($) {
	/**
	 * jquery getRangeAt function
	 */
	$.fn.getRangeAt = function() {
		var selectionParent = this;// element from the mouseup 
		var range = $.fn.range;// Reference to range object
		
		// Initialize variables
		range.ClearVariables();
		range.setRange();// gets Selection range
		
//		Verify what container the selection is allowed in.  		
// 		Check if First node Selection is in selectionParent
// 		Assume mouseUp is in the selectionParent (or last node)
		if (this[0] == document) {// Skips check if called like $().wrapSelection
			// Do nothing
		}
		else{
			var checkFirst = $(range.startContainer).parents().index(selectionParent);
			var checkLast = $(range.endContainer).parents().index(selectionParent);
			
			if ( checkFirst == -1 || checkLast == -1 ) {// restrict range to a specific container
				range.ClearVariables();
				return false;
			};
		}; 
		
//		// set commonAncestorContainer
//		var commonAncestorContainer = $(range.startContainer).parents().filter(function(){
//															return $(range.endContainer).parents().index(this) != -1;
//														})[0];
//		selRange.commonAncestorContainer	= commonAncestorContainer;
		
		return range;// returns range object, no chaining when getting Range
	};

	$.fn.wrapSelection = function(options) {
		var range = $.fn.range;
		var selectClass = 'sel_' + new Date().getTime();// Unique Class, created on each highlight
		var defaults = {
			fitToWord: true,
			wrapRange: false,
			selectClass: selectClass,
			regexElementBlockers: new RegExp(/^BR$/),// fitToWord Var
			regexWordCharacterBasic: new RegExp(/^[A-Za-z0-9'\-]$/),// fitToWord Var
			regexWordCharacterFull: new RegExp(/^[A-Za-z0-9':,\-]$/),// fitToWord Var
			regexWordPunc: new RegExp(/^[:,]$/),// fitToWord Var
			regexWordNumbers: new RegExp(/^[0-9]$/)// fitToWord Var
		};

		// build main options before element iteration
		var opts = $.extend({}, defaults, options);
		
		setWrapRange(this, opts.wrapRange);

		if (range.startContainer && range.endContainer){
			if(opts.fitToWord) FitToWord();

			SplitText();
			var myCount = doWrap();
			if (myCount) range.ClearAllRanges();
			else range.ClearVariables();
			
			// return opts.selectClass objects
			return $('.' + opts.selectClass);
		}
		else{
			return $([]);// return empty node
		};

		// Creates the range object	
		function setWrapRange(element, newRange){
	//		console.log('set-Element:', element);
			if(newRange)
				$.fn.range = newRange;	
			else
				$(element).getRangeAt();// test without parent call
		};
		
		function SplitText() {
			var range = $.fn.range;
			var myIsSameNode = (range.startContainer == range.endContainer);
			if (range.startContainer.nodeType == 3 && range.startOffset > 0) {
				var myNew = range.startContainer.splitText(range.startOffset);
				if (myIsSameNode) {//if they're the same node, we want to make sure to assign the end to the same as the start
					range.endContainer = myNew;
					range.endOffset = range.endOffset - range.startContainer.length;
				}
				range.startContainer = myNew;
				range.startOffset = 0;
			}
			if (range.endContainer.nodeType == 3 && range.endOffset < range.endContainer.length) {
				range.endContainer.splitText(range.endOffset);
				range.endOffset = range.endContainer.length;
			}
		};

// 		Adjusts the range object to go around the words
		function FitToWord() {
			var range = $.fn.range;
			var myStart = fitToStartWord(range.startContainer, range.startOffset, 'normal');
			var myEnd = fitToEndWord(range.endContainer, range.endOffset, 'normal');
			
			range.startContainer = myStart.container;
			range.startOffset = myStart.offset;
			range.endContainer = myEnd.container;
			range.endOffset = myEnd.offset;
		};
		
		function fitToEndWord (myContainer, myOffset, myType) {
			var myChar = '';
			if (myOffset > 0) myChar = myContainer.nodeValue.substr(myOffset - 1, 1);
			else {
				var myReverse = getPrevChar(myContainer, myOffset);
				//if the prev character is also a word, then assume it's part of same word and it's ok to go forward
				if (opts.regexWordCharacterFull.test(myReverse.character)) {
					myChar = myContainer.nodeValue.substr(myOffset, 0, 1);
					myOffset = 1;
				}
			}
			if (opts.regexWordCharacterBasic.test(myChar)) {//go forward
				if (myType == 'normal') {
					var myNormal = getNextChar(myContainer, myOffset - 1);
					if (opts.regexWordCharacterFull.test(myNormal.character)) {
						return fitToEndWord(myNormal.container, myNormal.offset + 1, 'normal');
					}
				}
				return {container: myContainer, offset: myOffset};
			}
			else if (myType == 'normal' && opts.regexWordPunc.test(myChar)) {//possibly go back or forward, depending on context
				var myNormal = getNextChar(myContainer, myOffset);
				if (opts.regexWordNumbers.test(myNormal.character)) return fitToEndWord(myNormal.container, myNormal.offset, 'normal');
				else return {container: myContainer, offset: myOffset - 1};
			}
			//otherwise go back
			var myReverse = getPrevChar(myContainer, myOffset - 1);
			if (myReverse.character.length == 1) return fitToEndWord(myReverse.container, myReverse.offset + 1, 'reverse');
			else return {container: myContainer, offset: myOffset};
		};

		function fitToStartWord(myContainer, myOffset, myType) {
			var myChar = myContainer.nodeValue.substr(myOffset, 1);
			if (opts.regexWordCharacterBasic.test(myChar)) {//go back
				if (myType == 'normal') {
					var myPrev = getPrevChar(myContainer, myOffset);
					if (opts.regexWordCharacterFull.test(myPrev.character)) {
						return fitToStartWord(myPrev.container, myPrev.offset, 'normal');
					}
				}
				return {container: myContainer, offset: myOffset};
			}
			else if (myType == 'normal' && opts.regexWordPunc.test(myChar)) {//possibly go back or forward, depending on context
				var myPrev = getPrevChar(myContainer, myOffset);
				if (opts.regexWordNumbers.test(myPrev.character)) return fitToStartWord(myPrev.container, myPrev.offset, 'normal');
			}
			var myNext = getNextChar(myContainer, myOffset);
			if (myNext.character.length == 1) return fitToStartWord(myNext.container, myNext.offset, 'reverse');
			else return {container: myContainer, offset: myOffset};
		};
	
		function getNextChar(myContainer, myOffset) {
			if (myOffset < 0) {
				var myPrevContainer = $.fn.wrapSelection.dom.GetPreviousTextNode(myContainer);
				if (myPrevContainer) {
					myContainer = myPrevContainer;
					myOffset = myContainer.length;
				}
			}
			if (myOffset < myContainer.length - 1) {
				return {container: myContainer, offset: myOffset + 1, character: myContainer.nodeValue.substr(myOffset + 1, 1)};
			}
			else {
				var myNext = $.fn.wrapSelection.dom.GetNextTextNode(myContainer, myContainer.parentNode);
				if (!myNext) return {container: myContainer, offset: myOffset, character: ''};
				var myNextElement = $.fn.wrapSelection.dom.GetNextSiblingElement(myContainer);
				while (myNextElement && $.fn.compareDocumentPosition(myNext, myNextElement) & 2) {
					if (myNextElement.nodeName.match(opts.regexElementBlockers)) return {container: myContainer, offset: myOffset, character: ''};
					myNextElement = $.fn.wrapSelection.dom.GetNextSiblingElement(myNextElement);
				}
				return {container: myNext, offset: 0, character: myNext.nodeValue.substr(0, 1)};
			}
		};

		function getPrevChar(myContainer, myOffset) {
			if (myOffset > 0) {
				return {container: myContainer, offset: myOffset - 1, character: myContainer.nodeValue.substr(myOffset - 1, 1)};
			}
			else {
				var myPrev = $.fn.wrapSelection.dom.GetPreviousTextNode(myContainer);
				if (!myPrev) return {container: myContainer, offset: myOffset, character: ''};
				var myPrevElement = $.fn.wrapSelection.dom.GetPreviousSiblingElement(myContainer);
				while (myPrevElement && $.fn.compareDocumentPosition(myPrev, myPrevElement) & 4) {
					if (myPrevElement.nodeName.match(opts.regexElementBlockers)) return {container: myContainer, offset: myOffset, character: ''};
					myPrevElement = $.fn.wrapSelection.dom.GetPreviousSiblingElement(myPrevElement);
				}
				return {container: myPrev, offset: myPrev.length - 1, character: myPrev.nodeValue.substr(myPrev.length - 1, 1)};
			}
		};

		function doWrap() {
			var myRange = $.fn.range;					
			var Spans = [];
			if (!myRange.startContainer || !myRange.endContainer) return false;

			var myNodes = myRange.GetContainedNodes();
			var iLength = myNodes.length;

			//myNodes is arranged by level, so everything at the same level can be surrounded by a <span>
			var myNodesSurrounded = 0;
			for (var i = 0; i < iLength; i++) {
				if (!myNodes[i][0]) continue;
				var myParent = myNodes[i][0].parentNode;
				var myParentName = myParent.nodeName;
				if (myParentName != 'DIV') {
					var mySpan = makeSpanElement();
					myParent.insertBefore(mySpan, myNodes[i][0]); //Firefox has bugs if we don't attach the span first; we can't just append it because we don't know where it goes in the parent
					Spans.push(mySpan);
				}
				for (var j = 0, jLength = myNodes[i].length; j < jLength; j++) {
					//this works assuming there aren't any block-level elements contained in the lower element; so it should work for P, but not for UL
					if (myParentName == 'DIV') {
						if (myNodes[i][j].nodeType != 1) continue;
						var myChildNodes = myNodes[i][j].childNodes;
						var mySpan = makeSpanElement();
						while (myChildNodes.length > 0) mySpan.appendChild(myChildNodes[0]);
						myNodes[i][j].appendChild(mySpan); //it's OK to do here because we're replacing the whole thing
						Spans.push(mySpan);
					}
					//appending automatically removes them
					else mySpan.appendChild(myNodes[i][j]);
					myNodesSurrounded++;
				}
			}
			return myNodesSurrounded;
		};

		function makeSpanElement() {
			var mySpan = document.createElement('span');
			mySpan.className = opts.selectClass;
			return mySpan;
		};
	};// END wrapSelection

	$.fn.range = {	
		onlySpacesMatch: new RegExp(/[^\t\r\n ]/),
		containedNodes: null,
		selection: null,
		commonAncestorContainer: null,
		startContainer: null,
		startOffset: null,
		endContainer: null,
		endOffset: null,
		collapsed: true,// default if null is true

		setRange : function(){
			if (window.getSelection) {
				this.selection = window.getSelection();
			}
			else if (document.selection) { // should come last; Opera!
				this.selection = document.selection.createRange();
			}

			if (this.selection.getRangeAt)
				var range = this.selection.getRangeAt(0);
			else { // Safari!
				var range = document.createRange();
				range.setStart(this.selection.anchorNode,this.selection.anchorOffset);
				range.setEnd(this.selection.focusNode,this.selection.focusOffset);
			}
			if (!range.toString().match(this.onlySpacesMatch)) return false;

			this.startContainer	= range.startContainer;
			this.startOffset	= range.startOffset;
			this.endContainer	= range.endContainer;
			this.endOffset		= range.endOffset;
			this.collapsed	= range.collapsed;
		},

		ClearAllRanges: function() {
			if (!$.fn.range.selection) return;
			//Firefox has bugs if you don't do both
			$.fn.range.selection.removeAllRanges();
			$.fn.range.ClearVariables();	
		},

		ClearVariables: function() {
			this.selection = null;
			this.commonAncestorContainer = null;
			this.containedNodes = null;
			this.startContainer = null;
			this.startOffset = null;
			this.endContainer = null;
			this.endOffset = null;
			this.collapsed = true;// Defualt is true if collapsed
		},

		GetContainedNodes: function() {
			return this.doGetContainedNodes();
		},

		doGetContainedNodes: function() {
			if (this.containedNodes) return this.containedNodes;
			if (!this.startContainer || !this.endContainer) return [];

			var myStart = this.startContainer;
			var myEnd = this.endContainer;
			var myNodes = new Array([]);
			var myNode = myStart;
			var myPosition = $.fn.compareDocumentPosition(myStart, myEnd);
			var myParent = myNode.parentNode;
			var i = 0;
			while ((myPosition & 4) || myPosition == 0) {//while the current node is before
				if (myPosition & 16) myNode = myNode.firstChild; //the current node contains the end node
				else {
					if (myParent != myNode.parentNode) {// we're at a new level (either up or down), so we need a new span
						i++;
						myNodes[i] = new Array;
						myParent = myNode.parentNode;
					}
					myNodes[i].push(myNode);
					myNode = $.fn.wrapSelection.dom.GetNextSiblingOrParent(myNode);
					if (myPosition == 0) break;
				}
				myPosition = $.fn.compareDocumentPosition(myNode, myEnd);
			}
			this.containedNodes = myNodes;
			return myNodes;
		}
	};

	// DOM Extend
	$.fn.wrapSelection.dom = {
		GetNextSiblingElement: function(myNode) {
			return $.fn.wrapSelection.dom.getElementOrder(myNode, 'next');
		},

		GetNextSiblingOrParent: function(myNode) {
			return $.fn.wrapSelection.dom.getSiblingOrParentOrder(myNode, 'next');
		},

		GetNextTextNode: function(myNode, myParent) {
			while (myNode = $.fn.wrapSelection.dom.getNodeOrder(myNode, myParent, 'next')) {
				if (myNode.nodeType == 3) return myNode;	
			}
			return myNode;
		},

		GetPreviousSiblingElement: function(myNode) {
			return this.getElementOrder(myNode, 'previous');
		},

		GetPreviousSiblingElement: function(myNode) {
			return this.getElementOrder(myNode, 'previous');
		},

		GetPreviousTextNode: function(myNode, myParent) {
			while (myNode = $.fn.wrapSelection.dom.getNodeOrder(myNode, myParent, 'previous')) {
				if (myNode.nodeType == 3) return myNode;	
			}
			return myNode;
		},

		getElementOrder: function(myNode, myType) {
			myType += 'Sibling';
			while (myNode[myType] && myNode[myType].nodeType != 1) {
				myNode = myNode[myType];
			}
			return myNode[myType];
		},

		getSiblingOrParentOrder: function(myNode, myOrder) {
			var mySibling = myOrder + 'Sibling';
			if (myNode[mySibling]) return myNode[mySibling];
			else if (myNode.parentNode) return this.getSiblingOrParentOrder(myNode.parentNode, myOrder)
			else return null;
		},

		getNodeOrder: function(myNode, myParent, myOrder) {//checkCurrent should usually only be called recursively
			if (typeof myParent == 'undefined') myParent = document.body;
			if (myNode.hasChildNodes()) return (myOrder == 'next') ? myNode.firstChild : myNode.lastChild;
			if (myNode == myParent) return null;
			var mySibling = (myOrder == 'next') ? 'nextSibling' : 'previousSibling';
			if (myNode[mySibling]) return myNode[mySibling];
			while (myNode = myNode.parentNode) {
				if (myNode == myParent) return null;
				if (myNode[mySibling]) return myNode[mySibling];
			}
			return null;
		}
	};

	// Integrate Internet Explorer Code
	if ($.browser.msie) {
		
		$.extend($.fn.range, {
			ClearAllRanges: function(){
				if (this.selection) 
					this.selection.empty(); //clear the current selection; we don't want it hanging around
				this.ClearVariables();
			},

			setRange : function(){
				this.selection = document.selection;
				var myRange = this.selection.createRange();
				var myText = myRange.text;
				if (!myText.length) return false;
				if (!myText.match(this.onlySpacesMatch)) return false; //if only whitespace, return

				var myStart = this.getInitialContainer(myRange.duplicate(), 'start');
					var myStartIndex = $.fn.wrapSelection.dom.SourceIndex(myStart.container, 'string');
				var myEnd = this.getInitialContainer(myRange.duplicate(), 'end');
					if (myStartIndex == $.fn.wrapSelection.dom.SourceIndex(myEnd.container, 'string')) myStart.container = myEnd.container;

				this.startContainer	= myStart.container;
				this.startOffset 	= myStart.offset;
				this.endContainer 	= myEnd.container;
				this.endOffset 		= myEnd.offset;
				this.collapsed 	= (myStart.container == myEnd.container && myStart.offset == myEnd.offset);

				myRange.select();// Fix Hightlight for IE that get's reset by getInitialContainer start node (myNode.insertData)
				return true;
			},

			getInitialContainer: function(myRange, myType) {
				if (myType == 'start') myRange.collapse(true); //collapse to start
				else myRange.collapse(false); //collapse to end
				var myParent = myRange.parentElement();
				myRange.pasteHTML('<span id="range-temp"></span>');
				var myTemp = $('#range-temp')[0];
				var myOffset = 0;
				var myNode = $.fn.wrapSelection.dom.GetNextTextNode(myTemp, myTemp.parentNode);
				if (!myNode) {
					myNode = $.fn.wrapSelection.dom.GetPreviousTextNode(myTemp, myTemp.parentNode);
					myOffset = myNode.length;
				}
				myTemp.parentNode.removeChild(myTemp);
				// Get's offset and merges adjacent textnodes together
				if (myType == "start") {
					if (myNode.previousSibling && myNode.previousSibling.nodeType == 3) {
						var myPrev = myParent.removeChild(myNode.previousSibling);
						myOffset += myPrev.length;
						myNode.insertData(0, myPrev.nodeValue);
					}
				}
				else {// End node
					if (myNode.previousSibling && myNode.previousSibling.nodeType == 3) {
						var myPrev = myNode.previousSibling;
						myOffset += myPrev.length;
						myParent.removeChild(myNode);
						myPrev.appendData(myNode.nodeValue);
						myNode = myPrev;
					}
				}
				return { container: myNode, offset: myOffset };
			}
		});

		$.extend($.fn.wrapSelection.dom, {
			SourceIndex: function(myNode, myType) {
				var myOut = [];
				do {
					var myOffset = 0;
					while (myNode.previousSibling) {
						myNode = myNode.previousSibling;
						myOffset++;
					}
					myOut.unshift(myOffset);
				}
				while (myNode = myNode.parentNode);
				if (myType && myType == 'string') 
					return myOut.join('.');
				return myOut;
			}
		});

	};
/** END Internet Explorer Code **/

// compareDocumentPosition - MIT Licensed, by ob. http://plugins.jquery.com/project/compareDocumentPosition
$.fn.compareDocumentPosition = function(node1, node2) {
	//Gecko, Opera have it built-in
	if ("compareDocumentPosition" in document.documentElement) {
		$.fn.compareDocumentPosition = function(node1, node2) {
			return node1.compareDocumentPosition(node2);
		};
	}
	//Internet Explorer
	else if ("sourceIndex" in document.documentElement && "contains" in document.documentElement) {
		$.fn.compareDocumentPosition = function(node1, node2) {
			if (node1 == node2) return 0;
			//if they don't have the same parent, there's a disconnect
			if (getRootParent(node1) != getRootParent(node2)) return 1;
			//use this if both nodes have a sourceIndex (text nodes don't)
			if ("sourceIndex" in node1 && "sourceIndex" in node2) {
				return comparePosition(node1, node2);
			}
			//document will definitely contain the other node
			if (node1 == document) return 20;
			else if (node2 == document) return 10;
			//get sourceIndexes to use for both nodes
			var useNode1 = getUseNode(node1), useNode2 = getUseNode(node2);
			//call this function again to get the result
			var result = comparePosition(useNode1, useNode2);
			//clean up if needed
			if (node1 != useNode1) useNode1.parentNode.removeChild(useNode1);
			if (node2 != useNode2) useNode2.parentNode.removeChild(useNode2);
			return result;

			//Compare Position - MIT Licensed, John Resig; http://ejohn.org/blog/comparing-document-position/
			//Already checked for equality and disconnect
			function comparePosition(node1, node2) {
				return (node1.contains(node2) && 16) +
					(node2.contains(node1) && 8) +
						(node1.sourceIndex >= 0 && node2.sourceIndex >= 0 ?
							(node1.sourceIndex < node2.sourceIndex && 4) +
								(node1.sourceIndex > node2.sourceIndex && 2) :
							1);
			}

			//get a node with a sourceIndex to use
			function getUseNode(node) {
				//if the node already has a sourceIndex, use that node
				if ("sourceIndex" in node) return node;
				//otherwise, insert a comment (which has a sourceIndex but minimal DOM impact) before the node and use that
				return node.parentNode.insertBefore(document.createComment(""), node);
			}
		};
	}
	else {
		//Safari and others; will work in IE
		//inspired by base2: http://code.google.com/p/base2/
		$.fn.compareDocumentPosition = function(node1, node2) {
			if (node1 == node2) return 0;
			if (getRootParent(node1) != getRootParent(node2)) return 1;
			//contains() only works if both are elements
			if (node1 == document
					|| ("contains" in node1 && "contains" in node2 && node1.contains(node2))) {
				return 20;
			}
			else if (node2 == document
					 || ("contains" in node1 && "contains" in node2 && node2.contains(node1))) {
				return 10;
			}
			return compareOffsetStrings(getOffsetString(node1), getOffsetString(node2));

			//takes the sortable string from getOffset
			function compareOffsetStrings(offset1, offset2) {
				//they're siblings or at the same depth
				if (offset1.length == offset2.length) {
					return (offset1 < offset2) ? 4 : 2;
					}
				//the first one is either a parent or at a shallower depth
				else if (offset1.length < offset2.length) {
					//truncate the longer one
					var offset2start = offset2.substr(0, offset1.length);
					//if they're the same at this point, we know node1 is a parent
					if (offset1 == offset2start) return 20;
					//call itself again now that they're the same length
					return compareOffsetStrings(offset1, offset2start);
					}
				else  {
					//flip the order of the arguments...
					var result = compareOffsetStrings(offset2, offset1);
					//...then shift the bits to get the correct result
					return (result & 4) ? result >> 1 : result << 1;
				}
			}

			//make a string that's sortable to determine a sourceIndex
			function getOffsetString(node) {
				var offsets = [];
				do {
					var offset = 0, prev = node;
					//count preceding siblings
					while (prev = prev.previousSibling) {
						offset++;
					}
					//get the total number of sibling nodes (before and after)
					var padLength = node.parentNode.childNodes.length.toString().length;
					var offsetLength = offset.toString().length;
					//zero-pad the offset to make sure the string compares properly
					if (padLength > offsetLength) {
						for (; offsetLength <= padLength; offsetLength++) {
							offset = "0" + offset;
						}
					}
					offsets.unshift(offset);
				}
				while ((node = node.parentNode) && node != document);
				//reverse the array to start the string at the top of the tree
				//return the final delimited string
				return offsets.join(".");
			}
		}
	}

	//node.ownerDocument gives the document object, which isn't the right info for a disconnect
	function getRootParent(node) {
		do { var parent = node; }
		while (node = node.parentNode);
		return parent;
	}

	//now that we've redefined the function during the first run, run it to get the actual value
	return $.fn.compareDocumentPosition(node1, node2);
};

// end of closure
})(jQuery);