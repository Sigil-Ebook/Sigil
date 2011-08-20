var block_elements = ["address", "blockquote", "del", "div", "dl", "fieldset", "form", "h1", "h2", "h3", "h4", "h5", "h6", "hr", "ins", "noscript", "ol", "p", "pre", "script", "table", "ul"];

// Obviously this assumes that the input is well-formed and there *is* a block node somewhere up the hierarchy
function get_block( node ) {
	var found = false;
	while( found == false ) {
		var i = 0;
		while( node.nodeName != block_elements[i] && i < block_elements.length ) {
			i++;
		}
		if( i == block_elements.length ) {
			node = node.parentNode;
		}
		else {
			found = true;
		}
	}
	
	return node;
};
