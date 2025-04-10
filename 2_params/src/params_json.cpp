#include <iostream>
#include <variant> // 用于存储多种类型
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class params_json
{
private:
    struct Address
    {
        std::string street;
        std::string number;
        std::string postcode;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Address, street, number, postcode);
    };

    struct Person
    {
        std::string name;
        int age;
        std::vector<Address> addresses;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Person, name, age, addresses);
    };
    struct ApiResult
    {
        bool success;
        std::string message;
        json data;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ApiResult, success, message, data);
    };

    Person person = {
        "John Doe",
        20,
        {{"Main St", "123", "12345"},
         {"Second St", "456", "67890"}}};

public:
    params_json() {};
    ~params_json() {};
    void serialization_test()
    {
        // 序列化为 JSON
        json j = person;
        // 输出查看
        std::cout << "---------------\n"
                  << "序列化Json后的样子，未添加格式\n"
                  << j << std::endl;
        // 输出查看
        std::cout << "---------------\n"
                  << "序列化Json后的样子\n"
                  << j.dump(4) << std::endl;
    }

    void deserialization_test()
    {
        json j2 = R"(
        {
            "name": "Jane Doe",
            "age": 25,
            "addresses":[
                {
                    "street":"jiangxia",
                    "number":"258",
                    "postcode":"54321"
                },
                {
                    "street":"wuchang",
                    "number":"369",
                    "postcode":"12345"
                }
            ]
        }
        )"_json;

        // 反序列化为 C++ 对象
        Person person2;
        j2.get_to(person2);
        // 输出查看
        std::cout << "---------------\n"
                  << "反序列化为 C++\n"
                  << "name: " << person2.name << std::endl
                  << "age: " << person2.age << std::endl
                  << "addresses: \n"
                  << "\tstreet1: " << person2.addresses[0].street << std::endl
                  << "\tnumber2: " << person2.addresses[1].number << std::endl;
    }

    void respond()
    {
        // 成功响应，携带数据
        ApiResult ar1;
        ar1.success = true;
        ar1.message = "success";
        ar1.data = person; // 数据可以是序列化前的，可也可以是序列化后的，推荐是结构体，未序列化成json的方便查看。
        json jar1 = ar1;

        // 错误响应
        ApiResult ar2;
        ar2.success = false;
        ar2.message = "A fatal error has occurred";
        ar2.data = nullptr;
        json jar2 = ar2;

        // 输出查看
        std::cout << "---------------\n"
                  << "成功响应，携带数据\n"
                  << jar1.dump(4) << std::endl;
        std::cout << "---------------\n"
                  << "错误响应\n"
                  << jar2.dump(4) << std::endl;
    }
};

class OneNET
{
private:
    // OneNET_Date_t 结构体
    template <typename T>
    struct OneNET_Date_t
    {
        T v = nullptr;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(OneNET_Date_t, v);
    };

    // OneNET不存在bool的类型，需要填写的是字符串，true/false代替
    struct OneNET_DATE
    {
        std::vector<OneNET_Date_t<bool>> falling;
        std::vector<OneNET_Date_t<double>> temperatrue;
        std::vector<OneNET_Date_t<std::string>> name;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(OneNET_DATE, falling, temperatrue, name);
    };

    struct OneNET_Frame
    {
        int id = 123;
        json dp;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(OneNET_Frame, id, dp);
    };

    struct OneNET_Result
    {
        int id;
        int err_code;
        std::string err_msg;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(OneNET_Result, id, err_code, err_msg);
    };

public:
    OneNET() {};
    ~OneNET() {};

    OneNET_Frame frame; // 存储数据的成员变量
    void set_id(const int id)
    {
        frame.id = id;
    }

    void set_date(bool falling = false, double temperatrue = 10.0, std::string name = "NULL")
    {
        OneNET_Date_t<bool> fall{falling};
        OneNET_Date_t<double> tem{temperatrue};
        OneNET_Date_t<std::string> na{name};
        OneNET_DATE date;

        date.falling.push_back(fall);
        date.temperatrue.push_back(tem);
        date.name.push_back(na);
        frame.dp = date;
    }

    void result_test()
    {
        json j = R"(
        {
            "id": 123
        }
        )"_json;
        // OneNET_Result back;
        // 数据需要一样才能转化，不能缺少或增加
        // j.get_to(back);

        json j2 = R"(
        {
            "id": 123,
            "err_code": 98,
            "err_msg": "illegal data"
        }
        )"_json;

        OneNET_Result back2;
        j2.get_to(back2);
        std::cout << "---------------\n"
                  << "反序列化为 C++\n"
                  << "id: " << back2.id << std::endl
                  << "err_code: " << back2.err_code << std::endl
                  << "err_msg: " << back2.err_msg << std::endl;
    }
};

int main()
{
    params_json te;
    te.serialization_test();
    // params_json test;
    // test.serialization_test();
    // test.deserialization_test();
    // test.respond();

    OneNET onenet;
    onenet.set_date(false, 13, "this_is_name");

    // 序列化为 JSON,就是字符串类型的一串
    json j = onenet.frame;
    // 输出查看
    std::cout << "---------------\n"
              << "序列化Json\n"
              << j.dump(4) << std::endl;

    onenet.result_test();

    return 0;
}
