/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Window that displays system information.
 **/
#ifndef TSUNAMI_LAB_SYSTEM_MONITOR_H
#define TSUNAMI_LAB_SYSTEM_MONITOR_H

namespace tsunami_lab
{
    namespace ui
    {
        class SystemMonitor;
    }
}

class tsunami_lab::ui::SystemMonitor
{
public:
    static void show();
};

#endif