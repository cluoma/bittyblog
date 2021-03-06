<!DOCTYPE html><html>
<head>
{{> head.m}}
</head>

{{> navbar.m}}

<div class="container">
<div class="row">
    <div class="col-sm-8 blog-main">

        {{> special_info_box.m}}

        {{#posts}}
        <div class="blog-post-preview-card">
            {{#rewrite}}
            <a href="/post/{{p_id}}"><img class="blog-post-preview-img" src="/images/{{thumbnail}}"></a>
            <a href="/post/{{p_id}}"><h2 class="blog-post-preview-title">{{title}}</h2></a>
            <p class="blog-post-meta">By <a href="/author/{{user_name_id}}">{{user_name}}</a>, {{time}}</p>
            <p>{{byline}}</p>
            <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="/tag/{{.}}">{{.}}</a> {{/tags}}</p>
            {{/rewrite}}
            {{^rewrite}}
            <a href="{{script_name}}?page={{page_name}}&id={{p_id}}"><img class="blog-post-preview-img" src="/images/{{thumbnail}}"></a>
            <a href="{{script_name}}?page={{page_name}}&id={{p_id}}"><h2 class="blog-post-preview-title">{{title}}</h2></a>
            <p class="blog-post-meta">By <a href="{{script_name}}?author={{user_name_id}}">{{user_name}}</a>, {{time}}</p>
            <p>{{byline}}</p>
            <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="{{script_name}}?tag={{.}}">{{.}}</a> {{/tags}}</p>
            {{/rewrite}}
        </div>
        {{/posts}}
        {{^posts}}
        <h2 class="blog-post-title" style="text-align: center;">No Posts Found</h2>
        {{/posts}}
        
        
        {{#nav_buttons}}
        <nav><ul class="pager">
        {{#rewrite}}
        {{#older}}
        <li style="float: left"><a href="{{#tag}}/tag/{{.}}?{{/tag}}{{^tag}}{{#search}}/search?search={{.}}{{/search}}{{^search}}?{{/search}}{{#search}}&{{/search}}{{/tag}}start={{.}}">Older</a></li>
        {{/older}}
        {{/rewrite}}
        {{^rewrite}}
        {{#older}}
        <li style="float: left"><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Older</a></li>
        {{/older}}
        {{/rewrite}}
    
        {{#rewrite}}
        {{#newer}}
        <li style="float: right"><a href="{{#tag}}/tag/{{.}}?{{/tag}}{{^tag}}{{#search}}/search?search={{.}}{{/search}}{{^search}}?{{/search}}{{#search}}&{{/search}}{{/tag}}start={{.}}">Newer</a></li>
        {{/newer}}
        {{/rewrite}}
        {{^rewrite}}
        {{#newer}}
        <li style="float: right"><a href="{{script_name}}?{{query_string_wo_start}}&start={{.}}">Newer</a></li>
        {{/newer}}
        {{/rewrite}}
        </ul></nav>
        {{/nav_buttons}}

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