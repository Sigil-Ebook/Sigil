// Uses get_ancestor.js

// From the current node, walk up the parent hierarchy until find a matching
// element matching one of the specified node names. If found return the value
// of the attribute matching the specified name if any.
function get_ancestor_attribute(startNode, targetNodeNames, attributeName) {
    var ancestorNode = get_ancestor(startNode, targetNodeNames);
    if ( ancestorNode != null) {
        return node.getAttribute(attributeName.toLowerCase());
    }
    return null;
};
