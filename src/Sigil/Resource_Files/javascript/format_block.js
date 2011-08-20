function format_block( startNode, element_name ) {
    var nodeContent = startNode.innerHTML;

    // Because people will moan like hell if these are left in
    nodeContent = nodeContent.replace( /xmlns=\"http:\/\/www.w3.org\/1999\/xhtml\"/g, "" );

    // Create a new tag with the desired name.
    var newBlock = document.createElement( element_name, "http://www.w3.org/1999/xhtml" );

    // Copy over all the attributes from the old block-level tag.
    var arrAttr = startNode.attributes;
    for(var j = 0; j < arrAttr.length; j++) {
        if(arrAttr[j].value != "" && arrAttr[j].value != "null") {
            var a = arrAttr[j].nodeName.toLowerCase();
            var v = arrAttr[j].nodeValue;
            
            switch(a) {
                case "class":
                    newBlock.className = v;
                    break;
                case "style":
                    newBlock.style.cssText = v;
                    break;
                default:
                    newBlock.setAttribute( a, v );
                    break;
            }
        }
    }
    
    // Inject the content from the old tag and replace the node.
    newBlock.innerHTML = nodeContent;
    
    return newBlock;
};
