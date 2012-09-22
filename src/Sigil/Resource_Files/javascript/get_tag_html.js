var node = document.getSelection().anchorNode;
var parent = node.parentNode;
var xml = "<" + parent.nodeName;
var arrAttr = parent.attributes;
for(var i = 0; i < arrAttr.length; i++) {
    if(arrAttr[i].value != "" && arrAttr[i].value != "null") {
        xml += " " + arrAttr[i].name + "='" + arrAttr[i].value + "'";
    }
}
xml += ">";
xml;