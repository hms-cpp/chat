#include<json.hpp>
#include<iostream>
#include<vector>
#include<map>
#include<string>

using json=nlohmann::json;

json func()
{
    json js;
    js["msg_type"]=2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]="hello";

    std::string s=js.dump();
    std::cout<<js<<std::endl;
    std::cout<<s.c_str()<<std::endl;
    return js;
}

void func1()
{
    json js;
    js["id"]={1,2,3,4};
    js["msg"]["name1"]="hello";
    js["msg"]["name2"]="world";
    std::cout<<js<<std::endl;
}

void func3()
{
    json js;

    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    js["list"]=v;

    std::map<int,std::string> m;
    m.insert(std::pair<int,std::string>(1,"黄山"));
    m.insert(std::pair<int,std::string>(2,"泰山"));
    m.insert(std::pair<int,std::string>(3,"泰山"));

    js["path"]=m;

    std::cout<<js<<std::endl;
    
}


int main()
{
    std::string s="11111";
    json buf=json::parse(s);
    std::cout<<buf<<std::endl;;
    return 0;
}

