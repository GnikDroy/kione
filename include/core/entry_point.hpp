#include "core/app.hpp"
#include <memory>

extern std::unique_ptr<k2::App> create_app();

#ifdef __linux__
int main(int, char**, char**){
    auto app = create_app();
    app->run();
}
#else 
#error "Your platform is not supported."
#endif