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
                <button type="submit" class="btn btn-default">
                    <!--<span class="glyphicon glyphicon-search"></span>-->
                    Go
                </button>
                </span>
                </input>
            </div>
            </form>
        </div>
    <!-- /.navbar-collapse -->
    </div>
<!-- /.container -->
</nav>