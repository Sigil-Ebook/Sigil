var replaceMark = "$ESCAPED_TEXT_HERE";

var selectRange = document.createRange();

var $parents = $("." + replaceMark).parent();

$("." + replaceMark).each(function (n) {
    if (n == 0) {
        selectRange.setStartBefore(this);
    }
    // Insert the previously stored html into the parent node before each element
    $(this).before($(this).data("undo"));

    if (n == 0) {
        if (this.previousSibling.nodeType == 3) { // TEXT
            selectRange.setEnd(this.previousSibling, this.previousSibling.nodeValue.length);
        }
        else { // it was wrapped in an inline tag
            selectRange.setEndAfter(this.previousSibling);
        }
        // Scroll to the cursor
        var from_top = window.innerHeight / 2;
        $.scrollTo(this, 0, { offset: { top: -from_top, left: 0} });
    }
    else {
        // Extend selection to include the other spans. Assumes that the wrapped set passes
        // its constituents in a fashion that monotonically advances through the text.
        if (this.previousSibling.nodeType == 3) { // TEXT
            selectRange.setEnd(this.previousSibling, this.previousSibling.nodeValue.length);
        }
        else {
            selectRange.setEndAfter(this.previousSibling);
        }
    }

    // Delete the span containing the replacement text.
    $(this).remove();
});

// Magic normalisation so the DOM regains sanity.
$parents.each(function () { this.normalize(); });

var selection = window.getSelection();
selection.removeAllRanges();
selection.addRange(selectRange);
