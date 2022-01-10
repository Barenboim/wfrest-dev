## Serving static files

```cpp
#include "wfrest/HttpServer.h"

using namespace wfrest;

int main()
{
    HttpServer svr;
    svr.Static("/static", "./www/static");

    svr.Static("/public", "./www");

    if (svr.start(8888) == 0)
    {
        getchar();
        svr.stop();
    } else
    {
        fprintf(stderr, "Cannot start server");
        exit(1);
    }
    return 0;
}
```