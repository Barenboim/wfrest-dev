#include "WFHttpMsg.h"
#include "WFWebServer.h"
#include "workflow/HttpUtil.h"
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <spdlog/spdlog.h>

using namespace wfrest;


void HttpReq::Body(const char **body, size_t *size) const
{
    // bool get_parsed_body(const void **body, size_t *size) const
    // https://stackoverflow.com/questions/16280814/static-cast-void-char-vs-static-cast-void-char
    get_parsed_body(reinterpret_cast<const void **>(body), size);
}

void HttpResp::String(const std::string &str)
{
    spdlog::info("resp string : {}", str);
    // bool append_output_body(const void *buf, size_t size);
    this->append_output_body(static_cast<const void *>(str.c_str()), str.size());
}

void HttpResp::Data(const void *data, size_t len, bool nocopy)
{
    // append_output_body_nocopy(const void *buf, size_t size)
    if (nocopy)
    {
        append_output_body_nocopy(data, len);
    } else
    {
        append_output_body(data, len);
    }
}

/*
We do not occupy any thread to read the file, but generate an asynchronous file reading task
and reply to the request after the reading is completed.

We need to read the whole data into the memory before we start replying to the message. 
Therefore, it is not suitable for transferring files that are too large.

todo : Any better way to transfer large file?
*/
void __pread_callback(WFFileIOTask *pread_task)
{
    FileIOArgs *args = pread_task->get_args();
    long ret = pread_task->get_retval();
    auto *resp = static_cast<HttpResp *>(pread_task->user_data);
    close(args->fd);
    if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
    {
        resp->set_status_code("503");
        resp->append_output_body("<html>503 Internal Server Error.</html>");
    } else
    {
        resp->append_output_body_nocopy(args->buf, ret);
    }
}

void HttpResp::File(const std::string &path)
{
    auto *server_task = WFWebServer::task_of(this);
    assert(server_task);

    int fd = open(path.c_str(), O_RDONLY);
    if (fd >= 0)
    {
        size_t size = lseek(fd, 0, SEEK_END);
        void *buf = malloc(size);
        WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(fd, buf,
                                                                    size, 0,
                                                                    __pread_callback);
        server_task->user_data = buf; /* to free() in callback() */
        pread_task->user_data = this;   /* pass resp pointer to pread task. */
        server_task->set_callback([](WFWebTask *server_task)
                                  {
                                      free(server_task->user_data);
                                  });
        **server_task << pread_task;
    } else
    {
        set_status_code("404");
        append_output_body("<html>404 Not Found.</html>");
    }
}

void HttpResp::set_status(int status_code)
{
    protocol::HttpUtil::set_response_status(this, status_code);
}
