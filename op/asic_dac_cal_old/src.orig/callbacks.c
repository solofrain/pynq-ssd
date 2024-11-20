#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

/* global variables from ca_stuff */
extern int NCH;
extern int NPOINTS;
extern float STEP;
extern long data[];
extern double fits[];
extern double x[];
extern double p0[];
extern char v1_l[], v1_h[], v2_l[], v2_h[];
extern int factor;
extern double mx;
extern double mn;
extern float dishi;
extern float dislo;
extern float cnt_time;
extern float guess;

extern void FitGraceinit();
extern void ScanGraceinit();
extern int makex();
extern void count();
extern void scan_levels();
extern void fitall();
extern float peval();
extern float residuals();
extern int fit();
extern void Correct();
extern int ResetTrims();


void
on_npnts_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
char tmp[64];

	sscanf(gtk_entry_get_text(entry),"%i",&NPOINTS);
	printf("Npoints changed: %i\n",NPOINTS);

}


void
on_cnt_time_activate                   (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%g",&cnt_time);
	printf("Count Time changed: %g\n",cnt_time);
}


void
on_dislo_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%g",&dislo);
	printf("Dislo changed: %g\n",dislo);
}


void
on_dishi_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%g",&dishi);
	printf("Dishi changed: %g\n",dishi);	
}


void
on_guess_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%g",&guess);
	printf("Guess changed: %g\n",guess);
	makex(guess);
}

void
on_step_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%g",&STEP);
	printf("Step changed: %g\n",STEP);
	makex(guess);
}


void
on_Scan_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Scan button pressed\n");
	scan_levels();
	
}


/*void
on_Correct_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Correct button pressed\n");
	fitall();
	Correct();
}
*/

void
on_Close_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Close button pressed\n");
	exit(0);
}


void
on_correct_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("correct button pressed\n");
	fitall();
	Correct();
}


void
on_Reset_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Reset button pressed\n");
	ResetTrims(32);
}

