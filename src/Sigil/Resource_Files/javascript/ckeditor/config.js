CKEDITOR.editorConfig = function(config)
{
    config.toolbarCanCollapse = false;
    config.toolbar = 'Full';
    config.toolbar_Full =
    [
        { name: 'clipboard', items : [ 'PasteText','PasteFromWord' ] },
        { name: 'basicstyles', items : [ 'Bold','Italic','Underline','Strike','Subscript','Superscript','-','RemoveFormat' ] },
        { name: 'format', items : [ 'NumberedList','BulletedList','-','Outdent','Indent','-','Blockquote','CreateDiv', ] },
        { name: 'align', items : ['JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock' ] },
        { name: 'direction', items : [ 'BidiLtr','BidiRtl' ] },
        { name: 'links', items : [ 'Link','Unlink','Anchor' ] },
        { name: 'insert', items : [ 'Table','HorizontalRule','SpecialChar','PageBreak' ] },
        { name: 'styles', items : [ 'Styles','Format','Font','FontSize' ] },
        { name: 'colors', items : [ 'TextColor','BGColor' ] },
    ];
    config.entities = false;
    config.docType = '&lt;!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"  "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"&gt;';
};
