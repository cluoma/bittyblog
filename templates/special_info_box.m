{{#special_info_box}}
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
<!-- Tag notification box -->
<div class="panel panel-default" style="border-color: #428bca">
    <div class="panel-heading" style="background-color: #428bca; color: #fff">Tag</div>
    <div class="panel-body">
        <p>Showing blog posts sorted under the tag: <b>{{.}}</b></p>
    </div>
</div>
{{/tag}}
{{#author}}
<!-- Author notification box -->
<div class="panel panel-default" style="border-color: #428bca">
    <div class="panel-heading" style="background-color: #428bca; color: #fff">Author</div>
    <div class="panel-body">
        <p>{{user_about}}<br><br>Showing blog posts written by: <b>{{user_name}}</b></p>
    </div>
</div>
{{/author}}
{{/special_info_box}}