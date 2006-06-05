#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "../../lib/vscopedevice.h"
VScope *vscope;


void
on_btnopen_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  vscope = openVScope();
}


void
on_btnsetcountermode_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  SetVScopeMode(vscope,MODE_COUNTER);

}


void
on_btnstart_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  StartVScope(vscope);

}


void
on_btnstop_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  StopVScope(vscope);

}


void
on_button5_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  GetVScopeFIFOLoad(vscope);
}


void
on_button6_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button7_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_btnreaddata_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  readVscopeData(vscope,NULL);
}

