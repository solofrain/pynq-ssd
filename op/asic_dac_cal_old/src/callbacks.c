#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include <cadef.h>

/* global variables from ca_stuff */
extern int NCH;
extern int NPOINTS;
extern float STEP;
extern long data[];
extern double fits[];
extern double x[];
extern double p0[];
extern char v1_l[], v1_h[], v2_l[], v2_h[];
extern float factor[];
extern double mx;
extern double mn;
extern float dishi;
extern float dislo;
extern float cnt_time;
extern float guess;
extern char Fname[];

extern void FitGraceinit();
extern void ScanGraceinit();
extern int makex();
extern void count();
extern void scan_levels();
extern void fitall();
extern void fit_plot();
extern double peval();
extern double residuals();
extern int fit();
extern void Correct();
extern int ResetTrims();
extern void Restore_dacs();
extern void exit();

int reset_val=32;
double UPRES,LOWRES,UPCEN,LOWCEN;


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
	Correct();
}


void
on_Reset_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	printf("Reset button pressed\n");
	ResetTrims(reset_val);
}


void
on_resetval_activate                   (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%i",&reset_val);
	if(reset_val>63) reset_val=63;
	if(reset_val<0) reset_val=0;
	printf("reset_val set to %i\n",reset_val);
}


void
on_dis_outs_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
/* disable all channels outside useable limits */
	Disable_outliers();
}


void
on_uppres_activate                     (GtkEntry        *entry,
                                        gpointer         user_data)
{
/* set upper limit of useful resolution */
	sscanf(gtk_entry_get_text(entry),"%lg",&UPRES);
	printf("UPRES changed: %lg\n",UPRES);

}


void
on_lowres_activate                     (GtkEntry        *entry,
                                        gpointer         user_data)
{
/* set lower limit of useful resolution */
	sscanf(gtk_entry_get_text(entry),"%lg",&LOWRES);
	printf("LOWRES changed: %lg\n",LOWRES);

}


void
on_uppcen_activate                     (GtkEntry        *entry,
                                        gpointer         user_data)
{
/* set upper limit of offset correction */
	sscanf(gtk_entry_get_text(entry),"%lg",&UPCEN);
	printf("UPCEN changed: %lg\n",UPCEN);

}


void
on_lowcen_activate                     (GtkEntry        *entry,
                                        gpointer         user_data)
{
/* set lower limit of offset correction */
	sscanf(gtk_entry_get_text(entry),"%lg",&LOWCEN);
	printf("LOWCEN changed: %lg\n",LOWCEN);
}


void
on_enable_all_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
/* enable all channels */
	Enable_all();
}


void
on_Show_remains_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
/* replot points indicating which ones are excluded */
	fit_plot(UPRES,LOWRES,UPCEN,LOWCEN);
}

unsigned int 
ca_rep_timer_expired ( int *data)

{
/* call channel access poll to maintain connections active */
	/*printf("poll\n");*/
	ca_poll();
	return(1);
}

void
on_Fit_clicked                         (GtkButton       *button,
                                        gpointer         user_data)
{
/* fit all curves to erf() function */
	fitall();
}




void
on_factor_activate                     (GtkEntry        *entry,
                                        gpointer         user_data)
{
/* set scale factor between volts and trim DAC units */
	sscanf(gtk_entry_get_text(entry),"%i",&factor);
	printf("Factor changed to %i\n",factor);
}


void
on_restore_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	Restore_dacs();	
}


void
on_fname_activate                      (GtkEntry        *entry,
                                        gpointer         user_data)
{
	sscanf(gtk_entry_get_text(entry),"%s",&Fname);
	printf("Filename changed to %s\n",Fname);
}

