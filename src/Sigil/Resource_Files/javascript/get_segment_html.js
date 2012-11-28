// uses external get_block.js

var node = document.getSelection().anchorNode;
var offset = document.getSelection().anchorOffset;
var splitNode = node.parentNode;
var splitOffset = 0;

function get_child_offset( node ) {
    for( var childIndex = 0, e = node; e = e.previousSibling; ++childIndex );
    return childIndex;
}

if ( node.nodeType == 3 ) {
    // We are in a #text node. Will want to split it at the caret position
    // if we are not at the end of the node text.
    if ( offset == 0 ) {
        // Splitting will mean the parent tag will become an empty tag.
        // Keep walking up parent nodes until either the left side will have
        // content, or we hit the body tag. This is to prevent leaving
        // an empty tag like <span></span> or <h1></h1> which if user does
        // not have a Clean/Tidy turned on will be left behind cruft.
        var parent = node.parentNode;
        while ( parent != null ) {
            if ( parent.childNodes[0] != node ) {
                // We are not the first child of this parent, so can split.
                break;
            }
            node = node.parentNode;
            parent = node.parentNode;
            if ( parent.nodeName == "body" ) {
                // We are going to split here rather than keep walking up.
                break;
            }
        }
        if ( parent != null ) {
            splitNode = parent;
            splitOffset = get_child_offset(node);
        }
    }
    else if ( offset <= node.length - 1 ) {
        node.splitText( offset );
        // Our range will be up to past this #text node
        splitOffset = get_child_offset(node) + 1;
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
                splitNode = parent;
                splitOffset = get_child_offset(node) + 1;
                break;
            }
            if ( parent == blockNode ) {
                // We reached the parent and must be the last node within it
                splitNode = parent.parentNode;
                splitOffset = get_child_offset(node) + 1;
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