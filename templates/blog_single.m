<!DOCTYPE html><html>
<head>
{{> head.m}}
</head>

{{> navbar.m}}

<div class="container">
<div class="row">
    <div class="col-sm-8 blog-main">

        {{#posts}}
        <div class="blog-post">
        {{#rewrite}}
        <a href="/post/{{p_id}}"><h2 class="blog-post-title">{{title}}</h2></a>
        <p class="blog-post-meta">By <a href="/author/{{user_name_id}}">{{user_name}}</a>, {{time}}</p>
        {{/rewrite}}
        {{^rewrite}}
        <a href="{{script_name}}?page={{page_name}}&id={{p_id}}"><h2 class="blog-post-title">{{title}}</h2></a>
        <p class="blog-post-meta">By <a href="{{script_name}}?author={{user_name_id}}">{{user_name}}</a>, {{time}}</p>
        {{/rewrite}}
        <p>{{&text}}</p>
        <hr>
        {{#rewrite}}
        <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="/tag/{{.}}">{{.}}</a> {{/tags}}</p>
        {{/rewrite}}
        {{^rewrite}}
        <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="{{script_name}}?tag={{.}}">{{.}}</a> {{/tags}}</p>
        {{/rewrite}}
        <hr>
        </div>
        <h2 >{{user_name}}: {{user_about}}</h2>
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

</div>

{{> footer.m}}

</html>