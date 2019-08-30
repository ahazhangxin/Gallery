#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
namespace image_system{

  static MYSQL* MySQLInit()
  {
    //使用 mysql API 来操作数据库
    //1.先创建一个 mysql 操作句柄
    MYSQL* mysql_ = mysql_init(NULL);
    //2.拿着句柄和数据库建立连接
    if(mysql_real_connect(mysql_,"127.0.0.1", "root","", "image_system", 3306, NULL, 0) == NULL)
    {
      printf("数据库连接错误！ %s\n", mysql_error(mysql_));
    }
    //3.设置编码格式
    mysql_set_character_set(mysql_, "utf8");
    return mysql_;
  }
  static void MySQLRelease(MYSQL* mysql)
  {
    mysql_close(mysql);
  }
  //操作数据库中的 image_table 这个表
  //此处的 Insert 等操作需要依赖大量的参数
  //所以我们用 JSON 封装参数
  class ImageTable{
    public:
      ImageTable(MYSQL* mysql)
        :mysql_(mysql)
      {}
      //使用 JSON 的原因
      //1.扩展更方便 
      //2.便于和服务器数据连通
      bool Insert(const Json::Value& image)
      {
        //JSON格式
        //
        char sql[4 *1024] = {0};
        sprintf(sql, "insert into image_table value(null, '%s', %d, '%s', '%s', '%s', '%s')",
            image["image_name"].asCString(),
            image["size"].asInt(), 
            image["upload"].asCString(),
            image["md5"].asCString(),
            image["type"].asCString(),
            image["path"].asCString());
        
        printf("[Insert sql] %s\n", sql);

        int ret = mysql_query(mysql_, sql);
        if(ret != 0)
        {
          printf("Insert 执行 SQL 失败! %s\n", mysql_error(mysql_));
          return false;
        }
        return true;
      }

      bool SelectAll(Json::Value * images)
      {
        char sql[4 *1024] = {0};
        sprintf(sql, "select * from image_table");
        int ret = mysql_query(mysql_, sql);
        if(ret != 0)
        {
          printf("SelectAll 执行 SQL 失败! %s\n", mysql_error(mysql_));
          return false;
        }
        //遍历结果集合，并把结果集写到 images 参数之中
        MYSQL_RES* result = mysql_store_result(mysql_);
        int rows = mysql_num_rows(result);
        for(int i = 0; i < rows; ++i)
        {
          MYSQL_ROW row = mysql_fetch_row(result);
          //数据库查出的每条记录都相当于是一个图片的信息
          //需要把这个信息转换成 JSON 格式
          Json::Value image;
          image["image_id"] = atoi(row[0]);
          image["image_name"] = row[1];
          image["size"] = atoi(row[2]);
          image["upload"] = row[3];
          image["md5"] = row[4];
          image["type"] = row[5];
          image["path"] = row[6];
          images->append(image);
        }
        //此处需要注意释放结果集合
        //否则会引起内存泄漏
        mysql_free_result(result);
        return true;
      }

      bool Selectone(int image_id, Json::Value* image_ptr)
      {
        char sql[ 1024 * 4 ] = {0};
        sprintf(sql, "select * from image_table where image_id = %d",image_id);
        int ret = mysql_query(mysql_, sql);
        if(ret != 0)
        {
          printf("SelectOne 执行 SQL 失败! %s\n", mysql_error(mysql_));
          return false;
        }
        MYSQL_RES* result = mysql_store_result(mysql_);
        int rows = mysql_num_rows(result);
        if(rows != 1)
        {
          printf("SelectOne 查询结果不是 1 条 ,实际上是 %d 条\n", rows);
          return false;
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        Json::Value image;
        image["image_id"] = atoi(row[0]);
        image["image_name"] = row[1];
        image["size"] = atoi(row[2]);
        image["upload"] = row[3];
        image["md5"] = row[4];
        image["type"] = row[5];
        image["path"] = row[6];
        *image_ptr = image;
        mysql_free_result(result);
        return true;
      }

      bool Delete(int image_id)
      {
        char sql[ 1024 * 4 ] = {0};
        sprintf(sql, "delete from image_table where image_id = %d", image_id);
        int ret = mysql_query(mysql_, sql);
        if(ret != 0)
        {
          printf("Delete 执行 SQL 失败! %s\n", mysql_error(mysql_));
          return false;
        }
        return true;
      }
    private:
      MYSQL* mysql_;
  };
}//end image_system
