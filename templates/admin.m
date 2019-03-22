<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <link href="/css/bittyblog.css" rel="stylesheet">
</head>

<body>

    <div class="admin-sidenav">
        <h3>{{navbar_title}}</h3>
        <a href="{{script_name}}?sid={{sid}}&c=pages">Pages</a>
        <a href="{{script_name}}?sid={{sid}}&c=posts">Posts</a>
        <a href="{{script_name}}?sid={{sid}}&c=media">Images</a>
    </div>

    <div class="admin-main">
    
    {{#category_pages}}
    <h1> Your Pages: </h1>
    <table>
        <tr><th>ID</th><th>ID Name</th><th>Proper Name</th><th>Tags</th></tr>
        {{#pages}}
        <tr>
            <td>{{id}}</td><td>{{id_name}}</td><td>{{name}}</td><td>{{#tags}}{{.}}, {{/tags}}</td>
        </tr>
        {{/pages}}
    </table>
    {{/category_pages}}


    {{#category_posts}}
    
    <h1> Your Posts: </h1>

    <!-- New Post Button -->
    <form action="{{script_name}}" method="GET">
        <input type="hidden" name="sid" value="{{sid}}" />
        <input type="hidden" name="c" value="posts" />
        <input type="hidden" name="a" value="new" />
        <button type="submit">New Post</button><br>
    </form> 

    <table>
        <tr><th>Edit</th><th>Published</th><th>Page</th><th>Title</th><th>Byline</th><th>Time</th><th>Delete</th></tr>
        {{#posts}}
        <tr>
            <td><a href="{{script_name}}?sid={{sid}}&p_id={{p_id}}">Edit</a></td>
            <td>{{#visible}}Published{{/visible}}{{^visible}}<b>Hidden</b>{{/visible}}</td><td>{{page}}</td><td>{{title}}</td><td>{{byline}}</td><td>{{time}}</td>
            <td>
                <form action="{{script_name}}?sid={{sid}}&c=posts&a=delete" method="POST">
                <input type="hidden" name="post_id" value="{{p_id}}" />
                <button type="submit">Delete</button>
                </form>
            </td>
        </tr>
        {{/posts}}
    </table>
    {{/category_posts}}

    <!-- Different types of forms for posts edit, and create -->
    {{#category_edit_posts}}
    {{#posts}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=posts&a=update" enctype="multipart/form-data">
        <div>
        <label for="post_thumbnail">Thumbnail: </label>
        <select name="post_thumbnail" id="post_thumbnail">
            {{#images}}
            <option value="{{filename}}" {{#selected}}selected{{/selected}}>{{filename}}</option>
            {{/images}}
        </select>
        </div>
        <div>
        <label for="post_page">Page: </label>
        <select name="post_page" id="post_page">
            {{#pages}}
            <option value="{{id}}" {{#selected}}selected{{/selected}}>{{name}}</option>
            {{/pages}}
        </select>
        </div>
        <div class="form-group"><label for="post_title">Title:</label><textarea name="post_title" class="form-control" rows="1" id="post_title">{{title}}</textarea></div><br>
        <div class="form-group"><label for="post_byline">Byline:</label><textarea name="post_byline" class="form-control" rows="1" id="post_byline">{{byline}}</textarea></div><br>
        <div class="form-group"><label for="post_text">Post:</label><textarea name="post_text" class="form-control" rows="20" id="post_text">{{text}}</textarea></div>
        <div class="form-group"><label for="post_tags">Tags:</label><textarea name="post_tags" class="form-control" rows="1" id="post_tags">{{#tags}}{{.}}, {{/tags}}</textarea></div>
        <div><input id="post_visible" type="checkbox" name="post_visible" value="Invisible" {{#visible}}checked{{/visible}}><label for="post_visible">Published</label><br></div>
        <div class="form-group hidden"><label for="post_id">Post ID:</label><textarea name="post_id" class="form-control" rows="1" id="post_id">{{p_id}}</textarea></div>
        <button type="submit">Submit</button>
    </form>
    {{/posts}}
    {{/category_edit_posts}}

    {{#category_new_posts}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=posts&a=new" enctype="multipart/form-data">
        <div>
        <label for="post_thumbnail">Thumbnail: </label>
        <select name="post_thumbnail" id="post_thumbnail">
            {{#images}}
            <option value="{{filename}}" {{#selected}}selected{{/selected}}>{{filename}}</option>
            {{/images}}
        </select>
        </div>
        <div>
        <label for="post_page">Page: </label>
        <select name="post_page" id="post_page">
            {{#pages}}
            <option value="{{id}}" {{#selected}}selected{{/selected}}>{{name}}</option>
            {{/pages}}
        </select>
        </div>
        <div class="form-group"><label for="post_title">Title:</label><textarea name="post_title" class="form-control" rows="1" id="post_title"></textarea></div><br>
        <div class="form-group"><label for="post_byline">Byline:</label><textarea name="post_byline" class="form-control" rows="1" id="post_byline"></textarea></div><br>
        <div class="form-group"><label for="post_text">Post:</label><textarea name="post_text" class="form-control" rows="20" id="post_text"></textarea></div>
        <div class="form-group"><label for="post_tags">Tags:</label><textarea name="post_tags" class="form-control" rows="1" id="post_tags"></textarea></div>
        <div><input id="post_visible" type="checkbox" name="post_visible" value="Invisible" {{#visible}}checked{{/visible}}><label for="post_visible">Published</label><br></div>
        <div class="form-group hidden"><label for="post_id">Post ID:</label><textarea name="post_id" class="form-control" rows="1" id="post_id"></textarea></div>
        <button type="submit">Submit</button>
    </form>
    {{/category_new_posts}}


    {{#category_media}}
    
    <h1> Your Media: </h1>

    <form role="form" action="{{script_name}}?sid={{sid}}&c=media&a=new" method="POST" enctype="multipart/form-data">
        <input type="file" name="media_upload" accept="image/*">
        <button type="submit">Upload Image</button><br>
    </form>

    <table>
        <tr><th>Delete</th><th>Image Name</th><th>Thumbnail</th>
        {{#images}}
        <tr>
            <td><form role="form" action="{{script_name}}?sid={{sid}}&c=media&a=delete" method="POST" enctype="multipart/form-data">
                    <input type="hidden" name="media_delete" value="{{filename}}" />
                    <button type="submit">Delete</button><br>
                </form>
            </td>
            <td>{{filename}}</td>
            <td><a href="/images/{{filename}}"><img src="/images/{{filename}}" style="height: 100px;width: 100px;"></a></td>
        </tr>
        {{/images}}
    </table>
    {{/category_media}}

    </div>

</body>
</html>