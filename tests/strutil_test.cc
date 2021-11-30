#include "StrUtil.h"

using namespace wfrest;

void test01()
{
    StringPiece str1("  name=\"avatar\"  ");
    StringPiece trim1 = StrUtil::trim(str1);
    // name="avatar"
    fprintf(stderr, "%s\n", trim1.data());
    // Watch here, delete right "
    // name="avatar
    fprintf(stderr, "%s\n", trim1.as_string().c_str());

    StringPiece tp = StrUtil::trim_pairs(trim1, R"(""'')");
    fprintf(stderr, "tp : %s\n", tp.data());
    fprintf(stderr, "tp str: %s\n", tp.as_string().c_str());
}

int main()
{

    test01();
}
