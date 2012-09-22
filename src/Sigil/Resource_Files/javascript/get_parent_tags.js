// uses get_block.js

var node = document.getSelection().anchorNode;
var blockNode = get_block(node);

var totalHtml = "";

function get_node_html(node) {
    var html = "<" + node.nodeName;
    var arrAttr = node.attributes;
    for(var i = 0; i < arrAttr.length; i++) {
        if(arrAttr[i].value != "" && arrAttr[i].value != "null") {
            html += " " + arrAttr[i].name + "='" + arrAttr[i].value + "'";
        }
    }
    html += ">";
    return html;
}

var totalHtml = "";

if (blockNode == node) {
    // Special case where user is not in a text node, like between <br/> tags
    totalHtml = get_node_html(blockNode);
}
else if (blockNode != null) {
    var currentNode = node.parentNode;
    while (currentNode != blockNode) {
        totalHtml = get_node_html(currentNode) + totalHtml;
        currentNode = currentNode.parentNode;
    };
    totalHtml = get_node_html(blockNode) + totalHtml;
}

totalHtml;