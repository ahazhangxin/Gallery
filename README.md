# Gallery
----
因为总有一些网站不支持图片上传功能，必须要使用图片链接，在我学习了数据库和网络的相关知识后，我决定向大家提供一个简易的图床
----
Because there are always some websites that do not support the image upload function, we must use image links. After I learned about the database and network, I decided to provide you with a simple graphics bed.
----

## 整体架构
----
核心就是一个 HTTP 服务器， 提供对图片的增删改查功能
同时搭配简单的页面辅助完成图片上传功能
使用 Json 作为数据交互格式
----
The core is an HTTP server, which provides the function of adding, deleting and checking pictures.
Simultaneously completes the picture upload function with the simple page assistance.
Using Json as data interaction format
----

### 数据库模块
将相关数据库操作封装起来
表的相关结构
```SQL
create table image_table{
	image_id int not null primary key auto_increment,
	image_name varchar(256),
	size int,
	upload varchar(50),
	md5 varchar(128),
	type varchar(128),
	path varchar(1024)
};
```
这个文件相当于 model 层.
只进行数据的基本 CURE ，不涉及到更复杂的数据加工
在这个库中封装了如下功能.
+ 插入单条数据 Insert
+ 查询单条数据 Selectone
+ 查询所有数据 SelectAll
+ 删除单个数据 Delete

### 服务器模块
此模块使用了 httplib 这个开源库
```C++
#include "httplib.h" 
int main() { 
 using namespace httplib; 
 Server server; 
 server.Get("/", [](const Request& req, Response& resp) { 
 (void)req; 
 resp.set_content("<html>hello</html>", "text/html"); 
 }); 
 server.set_base_dir("./wwwroot"); 
 server.listen("0.0.0.0", 9094); 
 return 0; 
}
```
在这个模块中实现了如下功能
+ 图片上传接口
+ 查看所有图片信息接口
+ 查看单个图片信息接口
+ 查看图片内容接口
+ 删除图片接口

使用 Postman 进行测试

----
### 尚未完成点
- [ ] 多个小图片拼接成一个大文件，提高存储效率
- [ ] 支持图片处理功能
- [ ] 防盗链
- [x] 引用计数方式保存多个相同的图片
  + 使用 MD5 作为存储名，同时在数据库中加入 引用计数 列
