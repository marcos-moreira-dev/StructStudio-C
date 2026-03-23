#include "app/windows_bootstrap.h"
#include "ui/main_window.h"

int main(void)
{
    ss_windows_bootstrap();
    return ss_main_window_run();
}
