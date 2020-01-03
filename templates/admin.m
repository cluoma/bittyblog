<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <link href="/css/bittyblog.css" rel="stylesheet">
</head>

<body>

    <div class="admin-sidenav">
        <h3>{{navbar_title}}</h3>
        <a href="{{script_name}}?sid={{sid}}&c=users">Users</a>
        <a href="{{script_name}}?sid={{sid}}&c=pages">Pages</a>
        <a href="{{script_name}}?sid={{sid}}&c=posts">Posts</a>
        <a href="{{script_name}}?sid={{sid}}&c=media">Images</a>
    </div>

    <div class="admin-main">
    
    <h1>Logged in as {{user}}</h1>

    {{#category_users}}
    <h1> Users: </h1>

    <!-- New Page Button -->
    <form action="{{script_name}}" method="GET">
        <input type="hidden" name="sid" value="{{sid}}" />
        <input type="hidden" name="c" value="users" />
        <input type="hidden" name="a" value="new" />
        <button type="submit">New User</button><br>
    </form>
    <br>

    <table>
        <tr><th>Edit</th><th>ID</th><th>Email</th><th>URL Name ID</th><th>Proper Name</th><th>Delete</th></tr>
        {{#users}}
        <tr>
            <td><a href="{{script_name}}?sid={{sid}}&c=users&a=update&id={{id}}">Edit</a></td>
            <td>{{id}}</td><td>{{email}}</td><td>{{name_id}}</td><td>{{name}}</td>
            <td>
                <form action="{{script_name}}?sid={{sid}}&c=users&a=delete" method="POST">
                <input type="hidden" name="id" value="{{id}}" />
                <button type="submit">Delete</button>
                </form>
            </td>
        </tr>
        {{/users}}
    </table>
    {{/category_users}}

    {{#category_new_users}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=users&a=new">
        <label for="email">Email:</label><br><textarea name="email" id="email" rows="1" cols="100"></textarea><br>
        <label for="password">Password:</label><br><textarea name="password" id="password" rows="1" cols="100"></textarea><br>
        <label for="name_id">URL Name ID:</label><br><textarea name="name_id" id="name_id" rows="1" cols="100"></textarea><br>
        <label for="name">Name:</label><br><textarea name="name" id="name" rows="1" cols="100"></textarea><br>
        <label for="about">About:</label><br><textarea name="about" id="about" rows="1" cols="100"></textarea><br>
        <div>
        <label for="thumbnail">Thumbnail: </label>
        <select name="thumbnail" id="thumbnail">
            {{#images}}
            <option value="{{filename}}" {{#selected}}selected{{/selected}}>{{filename}}</option>
            {{/images}}
        </select>
        </div>
        <button type="submit">Submit</button>
    </form>
    {{/category_new_users}}

    <!-- Different types of forms for posts edit, and create -->
    {{#category_edit_users}}
    {{#users}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=users&a=update">
        <label for="email">Email:</label><br><textarea name="email" id="email" rows="1" cols="100">{{email}}</textarea><br>
        <label for="name_id">URL Name ID:</label><br><textarea name="name_id" id="name_id" rows="1" cols="100">{{name_id}}</textarea><br>
        <label for="name">Name:</label><br><textarea name="name" id="name" rows="1" cols="100">{{name}}</textarea><br>
        <label for="about">About:</label><br><textarea name="about" id="about" rows="1" cols="100">{{about}}</textarea><br>
        <div>
        <label for="thumbnail">Thumbnail: </label>
        <select name="thumbnail" id="thumbnail">
            {{#images}}
            <option value="{{filename}}" {{#selected}}selected{{/selected}}>{{filename}}</option>
            {{/images}}
        </select>
        </div>
        <input id="id" name="id" type="hidden" value="{{id}}">
        <button type="submit">Submit</button>
    </form>
    {{/users}}
    {{/category_edit_users}}


    {{#category_pages}}
    <h1> Your Pages: </h1>

    <!-- New Page Button -->
    <form action="{{script_name}}" method="GET">
        <input type="hidden" name="sid" value="{{sid}}" />
        <input type="hidden" name="c" value="pages" />
        <input type="hidden" name="a" value="new" />
        <button type="submit">New Page</button><br>
    </form>
    <br>

    <table>
        <tr><th>Edit</th><th>ID</th><th>ID Name</th><th>Proper Name</th><th>Style</th><th>Tags</th><th>Delete</th></tr>
        {{#pages}}
        <tr>
            <td><a href="{{script_name}}?sid={{sid}}&page_id={{id}}">Edit</a></td>
            <td>{{id}}</td><td>{{id_name}}</td><td>{{name}}</td><td>{{style_name}}</td><td>{{#tags}}{{.}}, {{/tags}}</td>
            <td>
                <form action="{{script_name}}?sid={{sid}}&c=pages&a=delete" method="POST">
                <input type="hidden" name="page_id" value="{{id}}" />
                <button type="submit">Delete</button>
                </form>
            </td>
        </tr>
        {{/pages}}
    </table>
    {{/category_pages}}

    {{#category_new_pages}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=pages&a=new">
        <label for="page_name_id">URL Name ID:</label><br><textarea name="page_name_id" id="page_name_id" rows="1" cols="100"></textarea><br>
        <label for="page_name">Name:</label><br><textarea name="page_name" id="page_name" rows="1" cols="100"></textarea><br>
        <label for="page_tags">Tags:</label><br><textarea name="page_tags" id="page_tags" rows="1" cols="100"></textarea><br>
        <div>
        <label for="page_style">Style</label>
        <select name="page_style" id="page_style">
            {{#styles}}
            <option value="{{style}}" {{#selected}}selected{{/selected}}>{{style_name}}</option>
            {{/styles}}
        </select>
        </div>
        <button type="submit">Submit</button>
    </form>
    {{/category_new_pages}}

    <!-- Different types of forms for posts edit, and create -->
    {{#category_edit_pages}}
    {{#pages}}
    <form role="form" method="post" action="{{script_name}}?sid={{sid}}&c=pages&a=update">

        <label for="page_name_id">URL Name ID:</label><br><textarea name="page_name_id" id="page_name_id" rows="1" cols="100">{{id_name}}</textarea><br>
        <label for="page_name">Name:</label><br><textarea name="page_name" id="page_name" rows="1" cols="100">{{name}}</textarea><br>
        <label for="page_tags">Tags:</label><br><textarea name="page_tags" id="page_tags" rows="1" cols="100">{{#tags}}{{.}}, {{/tags}}</textarea><br>
        <input id="page_id" name="page_id" type="hidden" value="{{id}}">
        <div>
        <label for="page_style">Style</label>
        <select name="page_style" id="page_style">
            {{#styles}}
            <option value="{{style}}" {{#selected}}selected{{/selected}}>{{style_name}}</option>
            {{/styles}}
        </select>
        </div>
        <button type="submit">Submit</button>
    </form>
    {{/pages}}
    {{/category_edit_pages}}


    {{#category_posts}}
    
    <h1> Your Posts: </h1>

    <!-- New Post Button -->
    <form action="{{script_name}}" method="GET">
        <input type="hidden" name="sid" value="{{sid}}" />
        <input type="hidden" name="c" value="posts" />
        <input type="hidden" name="a" value="new" />
        <button type="submit">New Post</button><br>
    </form>
    <br>

    <table>
        <tr><th>Edit</th><th>Published</th><th>Page</th><th>Title</th><th>Byline</th><th>Time</th><th>Author</th><th>Delete</th></tr>
        {{#posts}}
        <tr>
            <td><a href="{{script_name}}?sid={{sid}}&p_id={{p_id}}">Edit</a></td>
            <td>{{#visible}}Published{{/visible}}{{^visible}}<b>Hidden</b>{{/visible}}</td><td>{{page}}</td>
            <td>{{title}}</td><td>{{byline}}</td><td>{{time}}</td><td>{{user_name}}</td>
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
    <!-- Edit -->
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
        <div>
        <label for="post_user_id">Author: </label>
        <select name="post_user_id" id="post_user_id">
            {{#users}}
            <option value="{{id}}" {{#selected}}selected{{/selected}}>{{name}}</option>
            {{/users}}
        </select>
        </div>
        <label for="post_title">Title:</label><br><textarea name="post_title" id="post_title" rows="1" cols="100">{{title}}</textarea><br>
        <label for="post_byline">Byline:</label><br><textarea name="post_byline" id="post_byline" rows="1" cols="100">{{byline}}</textarea><br>
        <label for="post_time">Time:</label><br><input type="number" min="0" name="post_time" id="post_time" rows="1" value="{{time_r}}"><br>
        <label for="post_text">Post:</label><br><textarea name="post_text" id="post_text" rows="20" cols="100">{{text}}</textarea><br>
        <label for="post_tags">Tags:</label><br><textarea name="post_tags" id="post_tags" rows="1" cols="100">{{#tags}}{{.}}, {{/tags}}</textarea><br>
        <label for="post_visible">Published</label><input id="post_visible" type="checkbox" name="post_visible" value="Invisible" {{#visible}}checked{{/visible}}><br>
        <input id="post_id" name="post_id" type="hidden" value="{{p_id}}">
        <button type="submit">Submit</button>
    </form>
    {{/posts}}
    {{#posts}}
    <br><br>
    <hr>
    <h3>Preview:</h3><br>
    <!-- Preview -->
    <div class="blog-post">
        <h2 class="blog-post-title">{{title}}</h2>
        <p class="blog-post-meta">{{time}}</p>
        <p>{{&text_formatted}}</p>
        <p style="clear: both; margin: 0px;"><b>Tags:</b> {{#tags}}<a class="blog-post-tag" href="#">{{.}}</a> {{/tags}}</p>
    </div>
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
        <div>
        <label for="post_user_id">Author: </label>
        <select name="post_user_id" id="post_user_id">
            {{#users}}
            <option value="{{id}}" {{#selected}}selected{{/selected}}>{{name}}</option>
            {{/users}}
        </select>
        </div>
        <label for="post_title">Title:</label><br><textarea name="post_title" id="post_title" rows="1" cols="100"></textarea><br>
        <label for="post_byline">Byline:</label><br><textarea name="post_byline" id="post_byline" rows="1" cols="100"></textarea><br>
        <label for="post_time">Time:</label><br><input type="number" min="0" name="post_time" id="post_time" rows="1"><br>
        <label for="post_text">Post:</label><br><textarea name="post_text" id="post_text" rows="20" cols="100"></textarea><br>
        <label for="post_tags">Tags:</label><br><textarea name="post_tags" id="post_tags" rows="1" cols="100"></textarea><br>
        <label for="post_visible">Published</label><input id="post_visible" type="checkbox" name="post_visible" value="Invisible"><br>
        <button type="submit">Submit</button>
    </form>
    {{/category_new_posts}}


    {{#category_media}}
    
    <h1> Your Media: </h1>

    <form role="form" action="{{script_name}}?sid={{sid}}&c=media&a=new" method="POST" enctype="multipart/form-data">
        <input type="file" name="media_upload" accept="image/*">
        <button type="submit">Upload Image</button><br>
    </form>
    <br>

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