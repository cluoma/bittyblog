<?xml version="1.0" encoding="UTF-8" ?>
<rss version="2.0">
<channel>

  <title>{{title}}</title>
  {{#rewrite}}<link>https://mywebsite.com/{{page_name}}</link>{{/rewrite}}{{^rewrite}}<link>https://mywebsite.com{{script_name}}?page={{page_name}}</link>{{/rewrite}}
  <description>My temporary description.</description>
  <language>en-ca</language>
  <category>Blogging</category>
  <copyright>Copyright {{current_year}} {{owner}}</copyright>

  {{#posts}}
  <item>
    <title>{{title}}</title>
    {{#rewrite}}<link>https://mywebsite.com/post/{{p_id}}</link>{{/rewrite}}{{^rewrite}}<link>https://mywebsite.com{{script_name}}?page={{page_name}}&id={{p_id}}</link>{{/rewrite}}
    {{#rewrite}}<guid>https://mywebsite.com/post/{{p_id}}</guid>{{/rewrite}}{{^rewrite}}<guid>https://mywebsite.com{{script_name}}?page={{page_name}}&id={{p_id}}</guid>{{/rewrite}}
    <description>{{byline}}</description>
    {{#tags}}<category>{{.}}</category>{{/tags}}
    <pubDate>{{time_rss}}</pubDate>
  </item>
  {{/posts}}

</channel>
</rss> 