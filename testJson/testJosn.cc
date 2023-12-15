#include <nlohmann/json.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <string>

using json = nlohmann::json;

using namespace std;

string serializationFunc1()
{
    json js;
    js["msg_type"]  = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello, what are you doing now?";
    string sendBuf = js.dump(); // json -> string
    cout << sendBuf << endl;
    // cout << js << endl;

    return sendBuf;
}

string serializationFunc2()
{
    json js;
    // 添加数组
    js["id"]  = {1, 2, 3, 4, 5};
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"] ["li si"]= "hello, what are you doing now?";
    // 上面等同于下面一次性添加数组对象
    js["msg"] = {{"zhang san", "hello warld"}, {"li si", "hello, what are you doing now?"}};
    string sendBuf = js.dump(); // json -> string
    cout << sendBuf << endl;
    // cout << js << endl;
    return sendBuf;
}

string serializationFunc3()
{
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"] = vec;
    // 直接序列话一个map容器
    map<int, string> m;
    m.emplace(1, "黄山");
    m.emplace(2, "华山");
    m.emplace(3, "泰山");
    js["path"] = m;
    string sendBuf = js.dump(); // json -> string
    cout << sendBuf << endl;
    // cout << js << endl;
    return sendBuf;
}

void parseFunc1(const string &jsonstr)
{
    json jsonbuf = json::parse(jsonstr);
    cout << jsonbuf["msg_type"] << endl;
    cout << jsonbuf["from"] << endl;
    cout << jsonbuf["to"] << endl;
    cout << jsonbuf["msg"] << endl;
}

void parseFunc2(const string &jsonstr)
{
    json jsonbuf = json::parse(jsonstr);
    cout << jsonbuf["id"] << endl;
    auto arr = jsonbuf["id"];
    cout << arr[2] << endl;
    cout << jsonbuf["msg"] << endl;
    auto msgjs = jsonbuf["msg"];
    cout << msgjs["zhang san"] << endl;
    cout << msgjs["li si"] << endl;
}

void parseFunc3(const string &jsonstr)
{
    json jsonbuf = json::parse(jsonstr);
    cout << jsonbuf["list"] << endl;
    auto arr = jsonbuf["list"];
    for(auto e : arr)
    {
        cout << e << endl;
    }
    cout << arr[1] << endl;

    cout << jsonbuf["path"] << endl;
    cout << jsonbuf["path"][1] << endl;
    map<int, string> m = jsonbuf["path"];
    auto m1 = jsonbuf["path"]; // 不会推导成为map,而是数组
    cout << m[1] << endl;
    cout << m1[1] << endl;
}

int main()
{
    // parseFunc1(serializationFunc1());
    // parseFunc2(serializationFunc2());
    parseFunc3(serializationFunc3());
    // serializationFunc2();
    // serializationFunc3();
    return 0;
}


