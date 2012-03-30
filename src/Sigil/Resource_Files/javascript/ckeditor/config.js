CKEDITOR.editorConfig = function( config )
{
	config.toolbar = 'Full';
	config.toolbar_Full =
	[
	    
	    { name: 'clipboard', items : [ 'Cut','Copy','Paste','PasteText','PasteFromWord' ] },
	    { name: 'basicstyles', items : [ 'Bold','Italic','Underline','Strike','Subscript','Superscript','-','RemoveFormat' ] },
    	{ name: 'paragraph', items : [ 'NumberedList','BulletedList','-','Outdent','Indent','-','Blockquote','CreateDiv', '-', 'JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock','-','BidiLtr','BidiRtl' ] },
    	{ name: 'links', items : [ 'Link','Unlink','Anchor' ] },
    	{ name: 'insert', items : [ 'Table','HorizontalRule','SpecialChar','PageBreak' ] },
    	{ name: 'styles', items : [ 'Styles','Format','Font','FontSize' ] },
    	{ name: 'colors', items : [ 'TextColor','BGColor' ] },
	]
};
