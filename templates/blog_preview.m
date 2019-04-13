<!DOCTYPE html><html>
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">
    <title>{{title}}</title>
    <!-- bittyblog core CSS -->
    <link href="/css/bittyblog.css" rel="stylesheet">
    <!-- Custom CSS -->
    <style>body {padding-top: 70px;}</style>
    <link rel="shortcut icon" type="image/png" href="/images/favicon.png"/>
</head>

<!-- Navigation -->
<nav class="navbar navbar-inverse navbar-fixed-top" role="navigation">
    <div class="container">
        <!-- Brand and toggle get grouped for better mobile display -->
        <input type="checkbox" id="navbar-toggle-cbox">
        <div class="navbar-header">
            <label for="navbar-toggle-cbox" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#bs-example-navbar-collapse-1" aria-expanded="false" aria-controls="navbar">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            </label>
            {{#rewrite}}
            <a class="navbar-brand" href="/">
            {{/rewrite}}
            {{^rewrite}}
            <a class="navbar-brand" href="{{script_name}}">
            {{/rewrite}}
            {{navbar_title}}
            </a>
        </div>
        <!-- Collect the nav links, forms, and other content for toggling -->
        <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
            
            <ul class="nav navbar-nav">
            {{#pages}}
            <li {{#active}}class="active"{{/active}}>
                {{#rewrite}}
                <a href="/{{id_name}}">{{name}}</a>
                {{/rewrite}}
                {{^rewrite}}
                <a href="{{script_name}}?page={{id_name}}">{{name}}</a>
                {{/rewrite}}
            </li>
            {{/pages}}
            </ul>

            {{#rewrite}}
            <form action="/search" method="get" class="navbar-form navbar-right" role="search">
            {{/rewrite}}
            {{^rewrite}}
            <form action="{{script_name}}" method="get" class="navbar-form navbar-right" role="search">
            {{/rewrite}}
            <div class="input-group">
                <input type="text" name="search" class="form-control" placeholder="Search">
                <span class="input-group-btn">
                <button type="submit" class="btn btn-default"><span class="glyphicon glyphicon-search"></span></button>
                </span>
                </input>
            </div>
            </form>
        </div>
    </div>
</nav>

<div class="container">
<div class="row">
    <div class="col-sm-8 blog-main">

        {{#search}}
        <!-- Search notification box -->
        <div class="panel panel-default" style="border-color: #428bca">
        <div class="panel-heading" style="background-color: #428bca; color: #fff">Search</div>
        <div class="panel-body">
        <p>Showing blog posts containing the search phrase: <b>{{.}}</b></p>
        </div>
        </div>
        {{/search}}
        {{#tag}}
        <!-- Search notification box -->
        <div class="panel panel-default" style="border-color: #428bca">
        <div class="panel-heading" style="background-color: #428bca; color: #fff">Tag</div>
        <div class="panel-body">
        <p>Showing blog posts sorted under the tag: <b>{{.}}</b></p>
        </div>
        </div>
        {{/tag}}

        {{#posts}}
        <div class="blog-post-preview-card">
            {{#rewrite}}
            <a href="/post/{{p_id}}"><img class="blog-post-preview-img" src="/images/{{thumbnail}}"></a>
            <a href="/post/{{p_id}}"><h2 class="blog-post-preview-title">{{title}}</h2></a>
            <p class="blog-post-meta">{{time}}</p>
            <p>{{byline}}</p>
            <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="/tag/{{.}}">{{.}}</a> {{/tags}}</p>
            {{/rewrite}}
            {{^rewrite}}
            <a href="{{script_name}}?page={{page_name}}&id={{p_id}}"><img class="blog-post-preview-img" src="/images/{{thumbnail}}"></a>
            <a href="{{script_name}}?page={{page_name}}&id={{p_id}}"><h2 class="blog-post-preview-title">{{title}}</h2></a>
            <p class="blog-post-meta">{{time}}</p>
            <p>{{byline}}</p>
            <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="{{script_name}}?tag={{.}}">{{.}}</a> {{/tags}}</p>
            {{/rewrite}}
        </div>
        {{/posts}}
        {{^posts}}
        <h2 class="blog-post-title" style="text-align: center;">No Posts Found</h2>
        {{/posts}}
        
    </div>
    <div class="col-sm-3 col-sm-offset-1 blog-sidebar">
        <!-- About box sidebar module-->
        <div class="sidebar-module sidebar-module-inset"><h4>About</h4>
            <p>Tell your readers about your blog</p>
        </div>
        <!-- Archives sidebar module-->
        <div class="sidebar-module"><h4>Archives</h4><ol class="list-unstyled">
            {{#archives}}
            {{#rewrite}}
            <li><a href="/archive/{{year}}/{{month}}">{{month_s}} {{year}} ({{post_count}})</a></li>
            {{/rewrite}}
            {{^rewrite}}
            <li><a href="{{script_name}}?month={{month}}&year={{year}}">{{month_s}} {{year}} ({{post_count}})</a></li>
            {{/rewrite}}
            {{/archives}}
        </ol></div>
    </div>
</div>

{{#nav_buttons}}
<nav><ul class="pager">
    {{#rewrite}}
    {{#older}}
    <li><a href="{{#tag}}/tag/{{.}}?{{/tag}}{{^tag}}{{#search}}/search?search={{.}}{{/search}}{{^search}}?{{/search}}{{#search}}&{{/search}}{{/tag}}start={{.}}">Older</a></li>
    {{/older}}
    {{/rewrite}}
    {{^rewrite}}
    {{#older}}
    <li><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Older</a></li>
    {{/older}}
    {{/rewrite}}
    
    {{#rewrite}}
    {{#newer}}
    <li><a href="{{#tag}}/tag/{{.}}?{{/tag}}{{^tag}}{{#search}}/search?search={{.}}{{/search}}{{^search}}?{{/search}}{{#search}}&{{/search}}{{/tag}}start={{.}}">Newer</a></li>
    {{/newer}}
    {{/rewrite}}
    {{^rewrite}}
    {{#newer}}
    <li><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Newer</a></li>
    {{/newer}}
    {{/rewrite}}
    </ul></nav>
{{/nav_buttons}}

</div>

<!-- Footer -->
<footer class="blog-footer">
    <p>Copyright {{current_year}} {{owner}}<br>
    Powered by <a href="http://github.com/cluoma/bittyblog">bittyblog</a>.</p>
    <p><a href="#">Back to top</a></p>
</footer>