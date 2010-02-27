var node = document.getSelection().anchorNode;
var caret_node = (node.nodeName == "#text" ? node.parentNode : node);

var range = document.createRange();
range.setStart( document.body, 0 );
range.setEnd( caret_node, caret_node.childNodes.length );  

new XMLSerializer().serializeToString( range.extractContents() );