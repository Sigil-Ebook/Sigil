// Uses get_ancestor.js

// From the current node, walk up the parent hierarchy until find a matching
// element matching one of the specified node names. If found set the value
// of the named attribute.
function set_ancestor_attribute(startNode, targetNodes, attributeName, attributeValue) {
    var ancestorNode = get_ancestor(startNode, targetNodes);
    if ( ancestorNode != null) {
        ancestorNode.setAttribute( attributeName, attributeValue );
        return true;
    }
    return false;
};
