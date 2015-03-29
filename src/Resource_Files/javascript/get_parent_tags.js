var node = document.getSelection().anchorNode;

function get_node_html(node) {
    var html = "<" + node.nodeName;
    var arrAttr = node.attributes;
    for( var i = 0; i < arrAttr.length; i++ ) {
        if( arrAttr[i].value != "" && arrAttr[i].value != "null" ) {
            html += " " + arrAttr[i].name + "='" + arrAttr[i].value + "'";
        }
    }
    html += ">";
    return html;
}

var totalHtml = "";

// If we are in a #text node jump to it's parent.
if ( node.nodeType == 3 ) {
    node = node.parentNode;
}

while ( node != null && ( node.nodeName.toLowerCase() != 'body' ) ) {
    totalHtml = get_node_html(node) + totalHtml;
    node = node.parentNode;
}

totalHtml;