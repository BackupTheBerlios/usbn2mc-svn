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
  char buffer[100];
  readVScopeData(vscope,buffer,100);
}


void
on_btn100us_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  SetVScopeSampleRate(vscope,SAMPLERATE_100US);
}


void
on_btn1ms_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  SetVScopeSampleRate(vscope,SAMPLERATE_1MS);
}


void
on_btn5us_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  SetVScopeSampleRate(vscope,SAMPLERATE_5US);
}


void
on_btn100ms_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  SetVScopeSampleRate(vscope,SAMPLERATE_100MS);
}


gboolean
on_main_destroy_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}

