var range = document.createRange();
range.setStart(element, 0);
range.setEnd(element, 0);
var selection = window.getSelection();
selection.removeAllRanges();
selection.addRange(range);
