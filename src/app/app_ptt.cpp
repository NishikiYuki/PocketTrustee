#include "app_ptt.hpp"

using namespace PocketTrustee;

App& App::GetApp() {
    static App app;
    return app;
}
