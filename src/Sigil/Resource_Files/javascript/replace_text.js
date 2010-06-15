
// Has to be run after the code in get_range.js
range.deleteContents();
var new_text_node = document.createTextNode("$ESCAPED_TEXT_HERE");
range.insertNode( new_text_node );

// We need to normalize the text nodes since they're screwed up now
range.startContainer.parentNode.normalize();
range.collapse( false );

// Expects new_selection.js to run after it here
