<!DOCTYPE html><html>
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content=""><meta name="author" content="">
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
            <a class="navbar-brand" href="{{script_name}}">{{navbar_title}}</a>
        </div>
        <!-- Collect the nav links, forms, and other content for toggling -->
        <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
            
            <ul class="nav navbar-nav">
            {{#pages}}
            <li {{#active}}class="active"{{/active}}>
                <a href="{{script_name}}?page={{id_name}}">{{name}}</a>
            </li>
            {{/pages}}
            </ul>

            <form action="{{script_name}}" method="get" class="navbar-form navbar-right" role="search">
            <div class="input-group">
                <input type="text" name="search" class="form-control" placeholder="Search">
                <span class="input-group-btn">
                <button type="submit" class="btn btn-default"><span class="glyphicon glyphicon-search"></span></button>
                </span>
                </input>
            </div>
            </form>
        </div>
    <!-- /.navbar-collapse -->
    </div>
<!-- /.container -->
</nav>

<div class="container" width="80%%">
    {{#posts}}
    <div class="panel panel-default" style="border-color: #428bca">
    <div class="panel-heading" style="background-color: #428bca; color: #fff">{{title}}</div>
    <div class="panel-body">
    <p>{{&text}}</p>
    </div>
    </div>
    {{/posts}}
    {{^posts}}
    <h2 class="blog-post-title" style="text-align: center;">No Posts Found</h2>
    {{/posts}}
</div>

{{#nav_buttons}}
<nav><ul class="pager">
    {{#older}}
    <li><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Older</a></li>
    {{/older}}
    {{#newer}}
    <li><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Newer</a></li>
    {{/newer}}
    </ul></nav>
{{/nav_buttons}}

</div>

<!-- Footer -->
<footer class="blog-footer">
    <p>Copyright {{current_year}} {{owner}}<br>
    Powered by <a href="http://github.com/cluoma/bittyblog">bittyblog</a>.</p>
    <p><a href="#">Back to top</a></p>
</footer>