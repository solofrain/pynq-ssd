#include <gtk/gtk.h>

extern int NCH; /* default value. Real value got from device later. */
extern int NPOINTS;
extern float STEP;
extern int NPLT;
extern long data; /*2 registers x 2 thresholds, up to 640 channels, up to 64-point scans */
extern double fits; /*2 registers x 2 thresholds, up to 640 channels, 4 parameters */
extern double x;
extern double p0;
extern char v1_l, v1_h, v2_l, v2_h, dis;
extern float factor;
extern double mx;
extern double mn;
extern float dishi;
extern float dislo;
extern float cnt_time;
extern float guess;
extern double fitdat,fitud;
extern char Fname;
extern int m, n, info, lwa, iwa, one;

float upres,lowres, upcen, lowcen;
void scan_levels();
void fitall();
void Correct();
void Disable_outliers();
void Enable_all();
void Restore_dacs();


GtkWidget *entry_npnts;
GtkWidget *entry_ctime;
GtkWidget *entry_lo_disable;
GtkWidget *entry_hi_disable;
GtkWidget *entry_pk_height;
GtkWidget *entry_step_size;
GtkWidget *entry_reset_val;
GtkWidget *entry_up_res_limit;
GtkWidget *entry_dn_res_limit;
GtkWidget *entry_up_cen_limit;
GtkWidget *entry_dn_cen_limit;

GtkWidget *button_scan;
GtkWidget *button_do_fit;
GtkWidget *button_correct;
GtkWidget *button_reset;
GtkWidget *button_dis_outliers;
GtkWidget *button_show_remaining;
GtkWidget *button_enable_all;
GtkWidget *button_dac_from_file;
GtkWidget *Quit;


    
int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
 
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "glade/window_main.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "applicationwindow1"));
    gtk_builder_connect_signals(builder, NULL);
    
    // get pointers to the entries
    entry_npnts = GTK_WIDGET(gtk_builder_get_object(builder, "entry_npnts"));
    entry_ctime = GTK_WIDGET(gtk_builder_get_object(builder, "entry_ctime"));
    entry_lo_disable = GTK_WIDGET(gtk_builder_get_object(builder, "entry_lo_disable"));
    entry_hi_disable = GTK_WIDGET(gtk_builder_get_object(builder, "entry_hi_disable"));
    entry_pk_height = GTK_WIDGET(gtk_builder_get_object(builder, "entry_pk_height"));
    entry_step_size = GTK_WIDGET(gtk_builder_get_object(builder, "entry_step_size"));
    entry_reset_val = GTK_WIDGET(gtk_builder_get_object(builder, "entry_reset_val"));
    entry_up_res_limit = GTK_WIDGET(gtk_builder_get_object(builder, "entry_up_res_limit"));
    entry_dn_res_limit = GTK_WIDGET(gtk_builder_get_object(builder, "entry_dn_res_limit"));
    entry_up_cen_limit = GTK_WIDGET(gtk_builder_get_object(builder, "entry_up_cen_limit"));
    entry_dn_cen_limit = GTK_WIDGET(gtk_builder_get_object(builder, "entry_dn_cen_limit"));

GtkWidget *entry_reset_val;
GtkWidget *entry_up_res_limit;
GtkWidget *entry_dn_res_limit;
GtkWidget *entry_up_cen_limit;
GtkWidget *entry_dn_cen_limit;

    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();
 
    return 0;
}

/* Actions taken when value entered */
 
void npnts_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_npnts);
    printf("%s\n", npnts);
    sscanf(npnts, "%i",&NPOINTS);
}

void ctime_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_ctime);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&cnt_time);
}

void lo_disable_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_lo_disable);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&dislo);
}
void hi_disable_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_hi_disable);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&dishi);
}

void pk_height_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_pk_height);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&guess);
}

void step_size_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_step_size);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&STEP);
}


void on_entry_reset_val_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_reset_val);
    printf("%s\n", npnts);
    sscanf(npnts, "%i",&n);
}

void on_entry_up_res_limit_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_up_res_limit);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&upres);
}

void on_entry_dn_res_limit_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_dn_res_limit);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&lowres);
}
    
void on_entry_up_cen_limit_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_up_cen_limit);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&upcen);
}

void on_entry_dn_cen_limit_activate()
{
    GtkEntryBuffer *npnts;
    
    npnts=gtk_entry_get_text(entry_dn_cen_limit);
    printf("%s\n", npnts);
    sscanf(npnts, "%f",&lowcen);
}

/* Button actions */

void on_button_scan_clicked()
{
    printf("Scanning\n");
    scan_levels();
}

void on_button_do_fit_clicked()
{
    printf("Fitting\n");
    fitall();
}

void on_button_correct_clicked()
{
    printf("Correcting\n");
    Correct();
}

void on_button_reset_clicked()
{
    printf("Reseting\n");
    ResetTrims();
}

void on_button_dis_outliers_clicked()
{
    printf("Disabled outliers\n");
    Disable_outliers();
}

void on_button_show_remaining_clicked()
{
    printf("Showing remainder\n");
}

void on_button_enable_all_clicked()
{
    printf("Enabling all\n");
    Enable_all();
}

void on_button_dac_from_file_clicked()
{
    printf("Loading DAC values from file\n");
    Restore_dacs();
}

 
/* called when window is closed */

void on_window_main_destroy()
{
    gtk_main_quit();
}
