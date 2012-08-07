// uses external get_block.js

var node = document.getSelection().anchorNode;
var caret_node = get_block( node );

var range = document.createRange();
range.setStart( document.body, 0 );
range.setEnd( caret_node, caret_node.childNodes.length );  

var serialiser = new XMLSerializer();
var contentForFirstFile = new String( serialiser.serializeToString( range.extractContents() ) );

// Explicitly remove the node as it may contain attrbutes that block this happening automatically
caret_node.parentNode.removeChild( caret_node );

contentForFirstFile.valueOf();