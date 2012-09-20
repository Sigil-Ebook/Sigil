// From the current node, walk up the parent hierarchy until find a matching
// element matching one of the specified node names.
function get_ancestor(startNode, targetNodeNames) {
	var found = false;
	while( found == false && node != null ) {
		for( var i = 0; i < 1; i++ ) {
            if ( node.nodeName.toLowerCase() == targetNodeNames[i].toLowerCase() ) {
                return node;
            }
        }
        node = node.parentNode;
	}
	return node;
};
