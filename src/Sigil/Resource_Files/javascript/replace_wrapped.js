// Expects to be preceeded by javascript that creates a variable called selectRange that
// defines the segment to be wrapped and replaced.

// create custom range object for wrapSelection
var replaceRange            = $.fn.range;
replaceRange.ClearVariables();
replaceRange.startContainer = selectRange.startContainer;
replaceRange.startOffset    = selectRange.startOffset;
replaceRange.endContainer   = selectRange.endContainer;
replaceRange.endOffset      = selectRange.endOffset;
replaceRange.collapsed      = selectRange.collapsed;


// Wrap the text to be replaced in a set of custom spans.
// This is done so we can operate on this text even if it extends over different
// inline tags.
var selectMarker = 'SigilReplace_' + new Date().getTime();
$().wrapSelection({fitToWord: false, selectClass: selectMarker, wrapRange: replaceRange});

// First, store the old contents so they can be undone, and then	
// insert the new text in the first element of the wrapped range and clear the rest.
$('.'+selectMarker).each(function(n) {
		if(n==0){
			$(this).data('undo', $(this).html());
			$(this).html("$ESCAPED_TEXT_HERE");
		}
		else {
			$(this).data('undo', $(this).html());
			// Assign an id so that this element isn't automatically deleted.
			$(this).attr("id",selectMarker+n);
			$(this).html('');
		}
	});

// We need to normalize the text nodes since they're screwed up now
selectRange.startContainer.parentNode.normalize();

// Set the cursor to point to the end of the replaced text.
selectRange.collapse( false );
var selection = window.getSelection();
selection.removeAllRanges();
selection.addRange(selectRange);

//Scroll to the cursor
var from_top = window.innerHeight / 2;
$.scrollTo( selectRange.startContainer, 0, {offset: {top:-from_top, left:0 } } );

// Return the unique class name used to identify these elements so the change can be undone.
selectMarker.valueOf();
