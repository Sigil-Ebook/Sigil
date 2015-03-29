var start_node = $START_NODE;
var end_node = $END_NODE;

var selectRange = document.createRange();
selectRange.setStart(start_node, $START_OFFSET);
selectRange.setEnd(end_node, $END_OFFSET);
