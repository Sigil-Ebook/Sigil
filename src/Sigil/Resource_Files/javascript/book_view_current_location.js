var startNode = document.getSelection().anchorNode;

// create a reverse function for jQuery
$.fn.reverse = [].reverse; 

var hierarchy = $( startNode ).parents().reverse().add( $( startNode ) ) ;

var hierarchy_string = hierarchy.map(function () { 
                                        if ( undefined != $( this ).parent().get( 0 ).tagName )
                                        {
                                            var first_part = $( this ).parent().get( 0 ).tagName + " ";
                                            
                                            // Webkit and QDomDocument count text nodes differently for some reason,
                                            // so we count non-text children until we reach the caret text node
                                            if ( this.nodeName != "#text"  )
                                                return first_part + $( this ).parent().children().index( this ); 
                                            else
                                                return first_part + jQuery.inArray( this, $( this ).parent().contents() );
                                        }
                                    })
                                .get().join(",");
                              
hierarchy_string += "," + startNode.nodeName + " -1" ;
