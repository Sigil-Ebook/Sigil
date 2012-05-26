CKEDITOR.editorConfig = function(config)
{
    config.toolbarCanCollapse = false;
    config.toolbar = 'Full';
    config.toolbar_Full =
    [
        { name: 'clipboard', items : [ 'PasteText','PasteFromWord' ] },
        { name: 'basicstyles', items : [ 'Bold','Italic','Underline','Strike','Subscript','Superscript','-','RemoveFormat' ] },
        { name: 'format', items : [ 'NumberedList','BulletedList','-','Outdent','Indent','-','Blockquote' ] },
        { name: 'align', items : ['JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock' ] },
        { name: 'direction', items : [ 'BidiLtr','BidiRtl' ] },
        { name: 'links', items : [ 'Link','Unlink','Anchor' ] },
        { name: 'insert', items : [ 'Table','HorizontalRule','SpecialChar' ] },
        { name: 'styles', items : [ 'Format' ] },
        { name: 'colors', items : [ 'TextColor','BGColor' ] },
    ];
    config.entities = false;
    config.format_tags = 'p;h1;h2;h3;h4;h5;h6';
    config.docType = '&lt;!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"  "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"&gt;';
};
