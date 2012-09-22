// uses external get_block.js

var node = document.getSelection().anchorNode;
var offset = document.getSelection().anchorOffset;
var splitNode = node.parentNode;
var splitOffset = 0;

if ( node.nodeType == 3 ) {
    // We are in a #text node. will want to split it at the caret position
    // if we are not at the end of the node text.
    if ( offset <= node.length - 1 ) {
        node.splitText( offset );
        // Our range will be up to past this #text node
        for( var childIndex = 0, e = node; e = e.previousSibling; ++childIndex );
        splitOffset = childIndex + 1;
    }
    else {
        // The selection should be treated as past this node.
        // We will walk up the hierarchy until either we find that:
        // (a) we are not the last child node of this parent, or
        // (b) we have reached the block node, and will split after that instead
        var blockNode = get_block( node );
        var parent = node.parentNode;
        while ( parent != null ) {
            if ( parent.childNodes[parent.childNodes.length - 1] != node ) {
                // We are not the last parent so will split before the next node
                for( var childIndex = 0, e = node; e = e.previousSibling; ++childIndex );
                splitNode = parent;
                splitOffset = childIndex + 1;
                break;
            }
            if ( parent == blockNode ) {
                // We reached the parent and must be the last node within it
                splitNode = parent.parentNode;
                for( var childIndex = 0, e = parent; e = e.previousSibling; ++childIndex );
                splitOffset = childIndex + 1;
                break;
            }
            node = parent;
            parent = node.parentNode;
        }
    }
}
else {
    // We are not in a #text node (such as between <br/> tags)
    splitNode = node;
    splitOffset = offset;
}

var range = document.createRange();
range.setStart( document.body, 0 );
range.setEnd( splitNode, splitOffset );

var serialiser = new XMLSerializer();
var contentForFirstFile = new String( serialiser.serializeToString( range.extractContents() ) );

var extractedText = contentForFirstFile.valueOf();

contentForFirstFile.valueOf();