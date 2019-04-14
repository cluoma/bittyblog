<!DOCTYPE html><html>
<head>
{{> head.m}}
</head>

{{> navbar.m}}

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

</div>

{{> footer.m}}

</html>