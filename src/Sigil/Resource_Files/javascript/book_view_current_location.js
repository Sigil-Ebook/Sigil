var node = document.getSelection().anchorNode;
var startNode = (node.nodeName == "#text" ? node.parentNode : node);

// create a reverse function for jQuery
$.fn.reverse = [].reverse; 

var hierarchy = $( startNode ).parents().reverse().add( $( startNode ) ) ;

var hierarchy_string = hierarchy.map(function () { 
                                        if ( undefined != $( this ).parent().get( 0 ).tagName )
                                        
                                            return $( this ).parent().get( 0 ).tagName + " " + $( this ).parent().children().index( this ); 
                                    })
                                .get().join(",");
                              
hierarchy_string += "," + startNode.nodeName + " -1" ;

