#include "SimpleClient.hpp"

int main()
{
    SimpleClient client("new_class@gmail.com", "test@localhost.com", "sending message from new cool class");
    client.sendMail();
    return 0;
}