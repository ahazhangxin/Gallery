#include<signal.h>
#include<jsoncpp/json/json.h>
#include"db.hpp"
#include"httplib.h"
#include<openssl/md5.h>
#include"util.hpp"



std::string StringMD5(const std::string& str) { 
  const int MD5LENTH = 16; 
  unsigned char MD5result[MD5LENTH]; 
  // 调用 openssl 的函数计算 md5 
  MD5((const unsigned char*)str.c_str(),str.size(),MD5result); 
  // 转换成字符串的形式方便存储和观察
  char output[1024] = {0}; 
  int offset = 0; 
  for (int i = 0; i < MD5LENTH; ++i) { 
    offset += sprintf(output + offset, "%x", MD5result[i]); 
  } 
  return std::string(output); 
}


const std::string base_path = "./image/";

MYSQL* mysql = NULL;

int main()
{
  using namespace httplib;
  using namespace image_system;
  Server server;

  //1.数据库客户端初始化和释放
  mysql = MySQLInit();
  signal(SIGINT, [](int){
      MySQLRelease(mysql);
      exit(0);
      });
  ImageTable image_table(mysql);


  //3.新增图片
  server.Post("/image",[&image_table](const Request& req, Response& resp)
      {
      Json::FastWriter writer;
      Json::Value resp_json;
      LOG(INFO) << "新增图片" << std::endl;
      //1.进行参数校验
      bool ret = req.has_file("filename");
      if(!ret){
      resp_json["ok"] = false;
      resp_json["reason"] = "req has no filename filed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      //2.构造 Json 格式数据，并调用数据库接口插入数据
      const auto& file = req.get_file_value("filename");
      const std::string& image_body = req.body.substr(file.offset, file.length);

      Json::Value image;
      image["image_name"] = file.filename;
      image["size"] = (int)file.length;
      image["upload"] = TimeUtil::FormatTime();
      image["md5"] = StringMD5(image_body);
      image["type"] = file.content_type;
      //为了防止重复，用 md5 作为文件名更合适
      image["path"] = base_path + file.filename;

      
      ret = image_table.Insert(image);
      if(!ret)
      {
        resp_json["ok"] = false;
        resp_json["reason"] = "insert db failed!\n";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //3.保存文件实体
      FileUtil::WriteFile(image["path"].asString(), image_body);

      //4.构造响应
      resp_json["ok"] = true;
      resp.set_content(writer.write(resp_json), "text/html");
      });

  //4.查看所有图片信息
  server.Get("/image", [&image_table](const Request& req, Response& resp){
      (void)req;
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value resp_json;
      LOG(INFO) << "查看所有图片信息: " <<std::endl;

      //1.调用数据库接口查询数据
      Json::Value images;
      bool ret = image_table.SelectAll(&images);
      if(!ret)
      {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectAll db failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      //2.构造响应结果
      resp.set_content(writer.write(images), "application/json");
      return;
  });

  //5.查看图片元信息
  server.Get(R"(/image/(\d+))", [&image_table](const Request& req, Response& resp){
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value resp_json;

      //1.获取到图片 id
      int image_id = std::stoi(req.matches[1]);
      LOG(INFO) << "查看图片信息" << image_id << std::endl;

      //2.调用数据库接口查询数据
      Json::Value image;
      bool ret = image_table.Selectone(image_id, &image);
      if(!ret)
      {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectAll failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }

      //3.构造响应结果
      resp.set_content(writer.write(image), "application/json");
      return;
  });

  //6.查看图片内容
  server.Get(R"(/image/show/(\d+))", [&image_table](const Request& req, Response& resp){
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value resp_json;

      //1.获取到图片 id
      int image_id = std::stoi(req.matches[1]);
      LOG(INFO) << "查看图片信息: " << image_id << std::endl;

      //2.调用数据库接口查询数据
      Json::Value image;
      bool ret = image_table.Selectone(image_id, &image);
      if(!ret)
      {
      resp_json["ok"] = false;
      resp_json["reason"] = "Selectone failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      std::string image_body;
      ret = FileUtil::ReadFile(image["path"].asString(), &image_body);
      if(!ret)
      {
        resp_json["ok"] = false;
        resp_json["reason"] = "path open failed!\n";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //3.构造响应结果
      resp.set_content(image_body, image["type"].asCString());
      return;
  });

  server.Delete(R"(/image/(\d+))", [&image_table](const Request& req, Response& resp){
      Json::Value resp_json;
      Json::FastWriter writer;

      //1.解析获取 blog_id
      //使用 matches[1] 就能获取到 blog_id
      //LOG(INFO) << req.matches[0] << "," << req.matches[1] << std::endl;
      int image_id = std::stoi(req.matches[1]);
      LOG(INFO) << "删除指定图片：" << image_id <<std::endl;

      //2.调用数据库接口删除图片
      bool ret = image_table.Delete(image_id);
      if(!ret)
      {
      resp_json["ok"] = false;
      resp_json["reason"] = "Delete failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      resp_json["ok"] = true;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
  });

  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 9000);
  return 0;
}
