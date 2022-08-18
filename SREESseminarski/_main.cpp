#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <tuple>
#include "systemModel.h"
#include "newtonRaphson.h"
#include "export.h"
#include "import.h"
#include "initList.h"
#include "entry.h"

SystemModel::SystemModel systemModel{10};

std::vector<std::tuple<int, double, double, std::string, double, double, int>> pvValues, pqValues, slackValues, sortedBuses;
std::vector<std::tuple<int, int, double, double, double, std::string, double, double, double, double>> lineValues;
std::vector<std::tuple<int, double, std::string, std::string, double, double>> batteryValues;
std::vector<std::tuple<int, int, double, double, double, double, std::string, double, double>> transValues;

std::vector<std::vector<std::tuple<int, double, double, std::string, double, double, int>>> buses;
std::vector<std::tuple<int, int>> coordinates;
std::vector<const char*> outputText;

const char *colors[] = {
  "DarkRed",
    "Red",
  "FireBrick",
  "Crimson",
  "IndianRed",
  "LightCoral",
  "Salmon",
  "DarkSalmon",
  "LightSalmon",
  "OrangeRed",
  "Tomato",
  "DarkOrange",
  "Coral",
  "Orange",
  "DarkKhaki",
  "Gold",
  "Khaki",
  "PeachPuff",
  "Yellow",
  "PaleGoldenrod",
  "Moccasin",
  "PapayaWhip",
  "LemonChiffon",
  "LightGoldenrodYellow",
  "LightYellow",
  "Maroon",
  "Brown",
  "SaddleBrown",
  "Sienna",
  "Chocolate",
  "DarkGoldenRod",
  "Peru",
  "RosyBrown",
  "Goldenrod",
  "SandyBrown",
  "Tan",
  "Burlywood",
  "Wheat",
  "NavajoWhite",
  "Bisque",
  "BlanchedAlmond",
  "Cornsilk",
  "DarkGreen",
  "Green",
  "DarkOliveGreen",
  "ForestGreen",
  "SeaGreen",
  "Olive",
  "OliveDrab",
  "MediumSeaGreen",
  "LimeGreen",
  "Lime",
  "SpringGreen",
  "MediumSpringGreen",
  "DarkSeaGreen",
  "MediumAquamarine",
  "YellowGreen",
  "LawnGreen",
  "Chartreuse",
  "LightGreen",
  "GreenYellow",
  "PaleGreen",
  "Teal",
  "DarkCyan",
  "LightSeaGreen",
  "CadetBlue",
  "DarkTurquoise",
  "MediumTurquoise",
  "Turquoise",
  "Aqua",
  "Cyan",
  "Aquamarine",
  "PaleTurquoise",
  "LightCyan",
  "Navy",
  "DarkBlue",
  "MediumBlue",
  "Blue",
  "MidnightBlue",
  "RoyalBlue",
  "SteelBlue",
  "DodgerBlue",
  "DeepSkyBlue",
  "CornflowerBlue",
  "SkyBlue",
  "LightSkyBlue",
  "LightSteelBlue",
  "LightBlue",
  "PowderBlue",
  "Indigo",
  "Purple",
  "DarkMagenta",
  "DarkViolet",
  "DarkSlateBlue",
  "BlueViolet",
  "DarkOrchid",
  "Fuchsia",
  "Magenta",
  "SlateBlue",
  "MediumSlateBlue",
  "MediumOrchid",
  "MediumPurple",
  "Orchid",
  "Violet",
  "Plum",
  "Thistle",
  "Lavender",
  "Black",
  "DarkSlateGray",
  "DimGray",
  "SlateGray",
  "Gray",
  "LightSlateGray",
  "DarkGray",
  "Silver",
  "LightGray",
  "Gainsboro",
  NULL
};

GtkListStore *store;
GtkApplication *app;

const double eps{ 1e-10 };

char pngPathName[512];
int startPoint = 0, startId, endId, capId, capX, capY;
int i, idx = 0, setFlag = 0, tabNumber, capColor;
GtkWidget *lblDevice, *tab, *btnSolve;
GtkWidget* win, *add, *treeview, *gridSlack, *gridPV, *gridPQ, *gridLine, *gridBattery, *gridTrans;
GtkWidget* area, *fixed,* pressedElement;
GtkWidget* pvList, *pqList, *lineList, *slackList, *batteryList, *transList;
static cairo_surface_t *surface = NULL;
static void setCapacitorValues();
static void pvTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void slackTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void pqTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void lineTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void batteryTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void transTable(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static GtkWidget *colorElement (const char *color, int op);
static void drawLine (GtkWidget *widget, std::string color);

double start_x, start_y, end_x, end_y;
int deviceSelected = 0, deviceIdx = 1, flagPressed = 0, xValDevice, yValDevice;


static void
on_save_response (GtkNativeDialog *dialog,
                  int        response)
{
  if (response == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
        GFile* file = gtk_file_chooser_get_file(chooser);
        const char* path = g_file_get_path(file);
        strcpy(pngPathName, path);
        g_object_unref(file);
        exportToTxt(path, systemModel, buses, lineValues, batteryValues, transValues);
    }
    g_object_unref(dialog);
}

static void exportToText(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    GtkFileChooserNative* dlg;
    dlg = gtk_file_chooser_native_new("Save Scheme", GTK_WINDOW(win), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save", "_Cancel");
    g_signal_connect (dlg, "response", G_CALLBACK (on_save_response),NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dlg));
    
}
int importFlag = 0;

static void on_open_response (GtkDialog *dialog, int response)
{
    SystemModel::SystemModel systemModel{10};
    if (response == GTK_RESPONSE_ACCEPT)
      {
          GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
          GFile* file = gtk_file_chooser_get_file(chooser);
          const char* path = g_file_get_path(file);
          slackValues.clear();
          pvValues.clear();
          pqValues.clear();
          lineValues.clear();
          batteryValues.clear();
          transValues.clear();
          buses.clear();
          if(importFromTxt(path, systemModel, &buses, &sortedBuses, &lineValues, &batteryValues, &transValues))
          {
              importFlag = 1;
              slackValues = buses[0];
              pvValues = buses[1];
              pqValues = buses[2];
              g_object_unref(file);
              g_object_unref(dialog);
              int k = 0, l = 0, m = 0;
              for(int i = 0; i < sortedBuses.size(); i++)
              {
                  int type = std::get<6>(sortedBuses[i]);
                  switch(type)
                  {
                      case 1:
                      {
                          GtkWidget* slack;
                          const char* color = std::get<3>(slackValues[k]).c_str();
                          double x,y;
                          x = std::get<4>(slackValues[k]);
                          y = std::get<5>(slackValues[k]);
                          slack = colorElement(color, 1);
                          gtk_fixed_put(GTK_FIXED(fixed), slack, x, y);
                          k++;
                          break;
                      }
                          
                      case 2:
                      {
                          GtkWidget* pv;
                          const char* color2 = std::get<3>(pvValues[l]).c_str();
                          double x3,y3;
                          x3 = std::get<4>(pvValues[l]);
                          y3 = std::get<5>(pvValues[l]);
                          pv = colorElement(color2, 2);
                          gtk_fixed_put(GTK_FIXED(fixed), pv, x3, y3);
                          l++;
                          break;
                      }
                          
                      case 3:
                      {
                          GtkWidget* pq;
                          const char* color3 = std::get<3>(pqValues[m]).c_str();
                          double x4,y4;
                          x4 = std::get<4>(pqValues[m]);
                          y4 = std::get<5>(pqValues[m]);
                          pq = colorElement(color3, 3);
                          gtk_fixed_put(GTK_FIXED(fixed), pq, x4, y4);
                          m++;
                          break;
                      }
                  }
              }
              for(int i = 0; i < transValues.size(); i++)
              {
                  GtkWidget* transformer;
                  const char* color = std::get<6>(transValues[i]).c_str();
                  double x,y;
                  int bus1, bus2;
                  bus1 = std::get<0>(transValues[i]);
                  bus2 = std::get<1>(transValues[i]);
                  x = std::get<7>(transValues[i]);
                  y = std::get<8>(transValues[i]);
                  transformer = colorElement(color, 6);
                  gtk_fixed_put(GTK_FIXED(fixed), transformer, x, y);
                  
                  for(int j = 0; j < 3; j++)
                  {
                      for(int k = 0; k < buses[j].size(); k++)
                      {
                          if(bus1 == std::get<0>(buses[j][k]))
                          {
                              start_x = std::get<4>(buses[j][k]) + 22;
                              start_y = std::get<5>(buses[j][k]) + 22;
                          }
                          if(bus2 == std::get<0>(buses[j][k]))
                          {
                              end_x = std::get<4>(buses[j][k]) + 22;
                              end_y = std::get<5>(buses[j][k]) + 22;
                              drawLine(area, std::get<5>(lineValues[0]));
                          }
                          
                      }
                  }
              }
              for(int i = 0; i < batteryValues.size(); i++)
              {
                  GtkWidget* battery;
                  int id = std::get<0>(batteryValues[i]);
                  const char* color = std::get<3>(batteryValues[i]).c_str();
                  double x,y;
                  x = std::get<4>(batteryValues[i]);
                  y = std::get<5>(batteryValues[i]);
                  for(int j = 0; j < 3; j++)
                  {
                      for(int k = 0; k < buses[j].size(); k++)
                      {
                          if(id == std::get<0>(buses[j][k]))
                          {
                              start_x = x + 22;
                              start_y = y + 10;
                              end_x = std::get<4>(buses[j][k]) + 22;
                              end_y = std::get<5>(buses[j][k]) + 22;
                              drawLine(area, std::get<3>(batteryValues[i]));
                          }
                      }
                  }
                  battery = colorElement(color, 5);
                  gtk_fixed_put(GTK_FIXED(fixed), battery, x, y);
              }
              for(int i = 0; i < lineValues.size(); i++)
              {
                  start_x = std::get<6>(lineValues[i])+27;
                  start_y = std::get<7>(lineValues[i])+22;
                  end_x = std::get<8>(lineValues[i])+27;
                  end_y = std::get<9>(lineValues[i])+22;
                  drawLine(area, std::get<5>(lineValues[i]));
              }
              return;
          }
          g_object_unref(file);
      }
      g_object_unref(dialog);
}


static void importFromText(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    GtkFileChooserNative* dlg;
    dlg = gtk_file_chooser_native_new("Save Scheme", GTK_WINDOW(win), GTK_FILE_CHOOSER_ACTION_OPEN, "_Open", "_Cancel");
    g_signal_connect (dlg, "response", G_CALLBACK (on_open_response),NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dlg));
    
}



static void closeTab()
{
    tabNumber = gtk_notebook_get_current_page(GTK_NOTEBOOK(tab));
    gtk_notebook_remove_page(GTK_NOTEBOOK(tab), tabNumber);
}

std::tuple<int, int> nearLine(double x, double y)
{
    double d;
    double x1, x2, y1, y2;
    for(int i = 0; i < coordinates.size(); i++)
    {
        x1 = std::get<0>(coordinates[i]);
        y1 = std::get<1>(coordinates[i]);
        for(int j = i + 1; j < coordinates.size(); j++)
        {
            x2 = std::get<0>(coordinates[j]);
            y2 = std::get<1>(coordinates[j]);
            d = (std::abs((x2-x1)*(y1-y)-(x1-x)*(y2-y1)))/(std::sqrt(std::pow(x2-x1,2) + std::pow(y2-y1,2)));
            if(d<15)
                return std::make_tuple(i + 1, j + 1);
        }
    }
    return std::make_tuple(-1, -1);
}

static void
drawSlack (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer        data)
{
  const char *color_name = (const char*) data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_set_line_width(cr, 2);
        cairo_rectangle(cr, 12, 7, 30, 30);
        cairo_stroke(cr);
        cairo_move_to(cr,12 ,12);
        cairo_line_to(cr, 17, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,12 ,17);
        cairo_line_to(cr, 22, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,12 ,22);
        cairo_line_to(cr, 27, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,12 ,27);
        cairo_line_to(cr, 32, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,12 ,32);
        cairo_line_to(cr, 37, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,12 ,37);
        cairo_line_to(cr, 42, 7);
        cairo_stroke(cr);
        
        cairo_move_to(cr,17 ,37);
        cairo_line_to(cr, 42, 12);
        cairo_stroke(cr);
        
        cairo_move_to(cr,22 ,37);
        cairo_line_to(cr, 42, 17);
        cairo_stroke(cr);
        
        cairo_move_to(cr,27 ,37);
        cairo_line_to(cr, 42, 22);
        cairo_stroke(cr);
        
        cairo_move_to(cr,32 ,37);
        cairo_line_to(cr, 42, 27);
        cairo_stroke(cr);
        
        cairo_move_to(cr,37 ,37);
        cairo_line_to(cr, 42, 32);
        cairo_stroke(cr);
            char num[10];
            sprintf(num, "%d", deviceIdx-1);
            cairo_select_font_face(cr, "Purisa",
                                   CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 9);
            cairo_move_to(cr, width/2-25, 15);
            cairo_show_text(cr, num);
            cairo_set_line_width(cr, 4);
            flagPressed = 0;

    }
}

static void
drawBattery (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer        data)
{
  const char *color_name = (const char*) data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_set_line_width(cr, 2);
        cairo_move_to(cr, 10, 10);
        cairo_line_to(cr, 45, 10);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 10, 16);
        cairo_line_to(cr, 45, 16);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 10, 16);
        cairo_line_to(cr, 45, 16);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 28, 16);
        cairo_line_to(cr, 28, 26);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 17, 26);
        cairo_line_to(cr, 38, 26);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 22, 30);
        cairo_line_to(cr, 33, 30);
        cairo_stroke(cr);
        
        cairo_move_to(cr, 27, 34);
        cairo_line_to(cr, 28, 34);
        cairo_stroke(cr);
    }
}

static void
drawPV (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer        data)
{
  const char *color_name = (const char*) data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_set_line_width(cr, 2);
      cairo_translate(cr, width/2, height/2);
      cairo_arc(cr, 0, 0, 18, 0, 2 * M_PI);
      cairo_stroke(cr);
    
      cairo_arc(cr, -5, 0, 5, M_PI, 0);
      cairo_stroke(cr);
        
      cairo_arc(cr, 5, 0, 5, 0, M_PI);
      cairo_stroke(cr);
        
        if(flagPressed)
        {
            char num[10];
            sprintf(num, "%d", deviceIdx-1);
            cairo_select_font_face(cr, "Purisa",
                                   CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 9);
            cairo_move_to(cr, width/2-5, -10);
            cairo_show_text(cr, num);
            cairo_set_line_width(cr, 4);
            flagPressed = 0;
        }
        
    }
}

static void
drawPQ (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer       data)
{
  const char *color_name = (const char*) data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_width(cr, 2);
      cairo_move_to(cr, width/2, 2);
        cairo_line_to(cr, width/2, 22);
        cairo_stroke(cr);
        
     cairo_move_to(cr, width/2, 22);
     cairo_line_to(cr, width/2 - 7, 22);
        cairo_stroke_preserve(cr);
        
        cairo_move_to(cr, width/2-7, 22);
        cairo_line_to(cr, width/2, 28);
        cairo_stroke_preserve(cr);
        
        cairo_move_to(cr, width/2, 28);
        cairo_line_to(cr, width/2 + 7, 22);
        cairo_stroke_preserve(cr);
        
        cairo_move_to(cr, width/2 + 7, 22);
        cairo_line_to(cr, width/2 , 22);
        cairo_stroke(cr);
        
        if(flagPressed)
        {
            char num[10];
            sprintf(num, "%d", deviceIdx-1);
            cairo_select_font_face(cr, "Purisa",
                                   CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 9);
            cairo_move_to(cr, width/2+5, 15);
            cairo_show_text(cr, num);
            cairo_set_line_width(cr, 4);
            flagPressed = 0;
        }
    }
}

static void
drawHLine (GtkDrawingArea *drawingarea,
           cairo_t        *cr,
           int             width,
           int             height,
           gpointer        data)
{
 const char *color_name = (const char*)data;
 GdkRGBA rgba;
 if (gdk_rgba_parse (&rgba, color_name))
   {
       gdk_cairo_set_source_rgba (cr, &rgba);
       cairo_set_line_width(cr, 2);
       cairo_move_to(cr, 10, 20);
       cairo_line_to(cr, 45, 20);
       cairo_stroke(cr);
   }
}

static void
drawLine (GtkWidget *widget, std::string color)
{
    cairo_t *cr;
    cr = cairo_create (surface);
    GdkRGBA rgba;
    
    if(color == "")
        gdk_rgba_parse (&rgba, colors[i]);
    else
    {
        const char* clr = color.c_str();
        gdk_rgba_parse (&rgba, clr);
    }

    gdk_cairo_set_source_rgba (cr, &rgba);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, end_x, end_y);
    cairo_stroke(cr);

    cairo_destroy (cr);
    gtk_widget_queue_draw (widget);
}

//static void
//lineWithId(GtkWidget *widget, std::string color)
//{
//    cairo_t *cr;
//    cr = cairo_create (surface);
//    GdkRGBA rgba;
//    const char* clr = color.c_str();
//    gdk_rgba_parse (&rgba, clr);
//    gdk_cairo_set_source_rgba (cr, &rgba);
//    cairo_set_line_width(cr, 2);
//    cairo_move_to(cr, start_x, start_y);
//    cairo_line_to(cr, end_x, end_y);
//    cairo_stroke(cr);
//
//    cairo_destroy (cr);
//    gtk_widget_queue_draw (widget);
//}


static void
drawTransformator (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer        data)
{
  const char *color_name = (const char*) data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_set_line_width(cr, 2);
      cairo_translate(cr, width/4 + 5, height/2);
      cairo_arc(cr, 0, 0, 14, 0, 2 * M_PI);
      cairo_stroke(cr);
    
      cairo_translate(cr,  width/4, 0);
      cairo_arc(cr, 0, 0, 14, 0, 2 * M_PI);
      cairo_stroke(cr);
    }
}


static GtkWidget *
colorElement (const char *color, int op)
{
    GtkWidget *area;
    area = gtk_drawing_area_new ();
 
    gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (area), 50);
    gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (area), 50);
    
    switch(op)
    {
        case 1:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawSlack, (gpointer) color, NULL);
            break;
        case 2:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawPV, (gpointer) color, NULL);
            break;
        case 3:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawPQ, (gpointer) color, NULL);
            break;
        case 4:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawHLine, (gpointer) color, NULL);
            break;
        case 5:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawBattery, (gpointer) color, NULL);
            break;
        case 6:
            gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), drawTransformator, (gpointer) color, NULL);
            break;
    }
    
  return area;
}
int getBusId (double line_x, double line_y)
{
    for(int i = 0; i < coordinates.size(); i++)
    {
        int x = std::get<0>(coordinates[i]);
        int y = std::get<1>(coordinates[i]);
        if(line_x < x + 30 && line_x > x - 30 && line_y > y - 30 && line_y < y + 30)
            return i + 1;
    }
    return 0;
}


static void setLineValues()
{
    GtkListStore *store;
    GtkTreeIter iter;
    
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lineList)));
    if(startId != endId && (startId != 0 && endId != 0))
    {

        lineValues.push_back(std::make_tuple(startId, endId, 0, 0, 0, colors[i], start_x, start_y, end_x, end_y));
        int i =(int) lineValues.size() - 1;
        const char *color = std::get<5>(lineValues[i]).c_str();
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, START_BUS, std::get<0>(lineValues[i]), END_BUS, std::get<1>(lineValues[i]), REAL_IMPEDANCE, std::get<2>(lineValues[i]),IMAG_IMPEDANCE,std::get<3>(lineValues[i]), SHUNT, std::get<4>(lineValues[i]), COLOR, color, -1);
    }
    if(startId == 0 && endId != 0)
    {
        capId = endId;
        setCapacitorValues();
    }
    else if(endId == 0 && startId != 0)
    {
        capId = startId;
        setCapacitorValues();
    }

}

static void
pressedLine (GtkGestureClick *gesture,
         int              n_press,
         double           x,
         double           y,
         GtkWidget       *fixed)
{
    if(idx == 3)
    {
        if(!startPoint)
        {
            start_x = x;
            start_y = y;
            startPoint = 1;
            startId = getBusId(start_x, start_y);
            
        }
        else
        {
            end_x = x;
            end_y = y;
            endId = getBusId(end_x, end_y);
            startPoint = 0;
            //gtk_widget_set_size_request(area, 1100, 700);
            drawLine(area, "");
            setLineValues();
        }
    }
}

static void setSlackValues()
{
    GtkTreeIter iter;
    GtkListStore *storeSlack;
    storeSlack = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(slackList)));
    int val = deviceIdx - 1;

    slackValues.push_back(std::make_tuple(val, 0, 0, colors[i], xValDevice, yValDevice, 1));
    int i = (int)slackValues.size() - 1;
    const char *color = std::get<3>(slackValues[i]).c_str();
    gtk_list_store_append(storeSlack, &iter);
    gtk_list_store_set (storeSlack, &iter, BUS_ID, std::get<0>(slackValues[i]), VOLTAGE_MAGNITUDE, std::get<1>(slackValues[i]), VOLTAGE_PHASE, std::get<2>(slackValues[i]), COLOR, color, -1);
    sortedBuses.push_back(slackValues[i]);
}

static void setPvValues()
{
    GtkTreeIter iter;
    GtkListStore *storePv;

    storePv = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pvList)));
    int val = deviceIdx - 1;
    pvValues.push_back(std::make_tuple(val, 0, 0, colors[i], xValDevice, yValDevice, 2));
    int k = (int)pvValues.size() - 1;
    const char *color = std::get<3>(pvValues[k]).c_str();
    gtk_list_store_append(storePv, &iter);
    gtk_list_store_set (storePv, &iter, BUS_ID, std::get<0>(pvValues[k]), VOLTAGE_MAGNITUDE, std::get<1>(pvValues[k]), ACTIVE_POWER, std::get<2>(pvValues[k]),COLOR, color, -1);
    sortedBuses.push_back(pvValues[k]);
}

static void setPqValues()
{
    GtkListStore *store;
    GtkTreeIter iter;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pqList)));

    int val = deviceIdx - 1;
    pqValues.push_back(std::make_tuple(val, 0, 0, colors[i], xValDevice, yValDevice, 3));
    int i = (int) pqValues.size() - 1;
    const char *color = std::get<3>(pqValues[i]).c_str();
    gtk_list_store_append(store, &iter);
    gtk_list_store_set (store, &iter, BUS_ID, std::get<0>(pqValues[i]), ACTIVE_POWER, std::get<1>(pqValues[i]), REACTIVE_POWER, std::get<2>(pqValues[i]),COLOR, color, -1);
    sortedBuses.push_back(pqValues[i]);
}
static void setCapacitorValues()
{
    GtkTreeIter iter;
    GtkListStore *store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(batteryList)));
    batteryValues.push_back(std::make_tuple(capId, 0, "Delta", colors[capColor], capX, capY));
    int i = (int)batteryValues.size() - 1;
    const char *config = std::get<2>(batteryValues[i]).c_str();
    const char *color = std::get<3>(batteryValues[i]).c_str();
    gtk_list_store_append(store, &iter);
    gtk_list_store_set (store, &iter, BUS_ID, std::get<0>(batteryValues[i]), CAPACITY, std::get<1>(batteryValues[i]), CONFIGURATION, config, COLOR, color, -1);

}

static void setTransValues()
{
    GtkListStore *store;
    GtkTreeIter iter;
    
    int startId, endId;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(transList)));
    std::tuple<int, int> index = nearLine(xValDevice, yValDevice);
    startId = std::get<0>(index);
    endId = std::get<1>(index);
    if(startId >= 0 && endId >= 0)
    {
        transValues.push_back(std::make_tuple(startId, endId, 0, 0, 0,0, colors[i], xValDevice, yValDevice));

        int j =(int) transValues.size() - 1;
        const char *color = std::get<6>(transValues[j]).c_str();
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, START_BUS, std::get<0>(transValues[j]), END_BUS, std::get<1>(transValues[j]), REAL_IMPEDANCE, std::get<2>(transValues[j]),IMAG_IMPEDANCE,std::get<3>(transValues[j]), SHUNT_REAL, std::get<4>(transValues[j]),SHUNT, std::get<5>(transValues[j]), COLOR, color, -1);
    }
    
    for(int i = 0; i < lineValues.size(); i++)
    {
        for(int j = 0; j < transValues.size(); j++)
        {
            if((std::get<0>(lineValues[i]) == std::get<0>(transValues[j]) || std::get<0>(lineValues[i]) == std::get<1>(transValues[j])) && (std::get<1>(lineValues[i]) == std::get<1>(transValues[j]) || std::get<1>(lineValues[i]) == std::get<0>(transValues[j])))
                lineValues.erase(lineValues.begin() + i);
        }
    }
}

static void
pressed (GtkGestureClick *gesture,
         int              n_press,
         double           x,
         double           y,
         GtkWidget       *area)
{
    if(setFlag)
    {
        if(idx == 3)
            return;

            xValDevice = x;
            yValDevice = y;
            GtkWidget *pressedElement;
            pressedElement = colorElement(colors[i], idx + 1);
         
            gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (pressedElement), 50);
            gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (pressedElement), 50);

            gtk_fixed_put(GTK_FIXED(area), pressedElement, x - 27, y - 20);
            if(idx != 4 && idx != 5)
            {
                coordinates.push_back(std::make_tuple(x,y));
                deviceIdx++;
            }
            else if(idx == 4)
            {
                capX = x;
                capY = y;
                capColor = i;
            }
            flagPressed = 1;
           switch(idx)
            {
                case 0:
                    setSlackValues();
                    break;
                case 1:
                    setPvValues();
                    break;
                case 2:
                    setPqValues();
                    break;
                case 5:
                    setTransValues();
                    break;
            }
            setFlag = 0;
        

    }
}


static void
activateDevice(GtkWidget *flowbox, GtkWidget *area, GtkWidget *grid)
{
    setFlag = 1;
    idx = 0;
    GtkFlowBoxChild *ref;
    while(1)
    {
        ref = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flowbox), idx);
        if(GTK_WIDGET(ref) == area)
            break;
        idx++;
    }
    const char* deviceType;
    switch(idx)
    {
        case 0:
            deviceType = "Slack Bus";
            break;
        case 1:
            deviceType = "PV Bus";
            break;
        case 2:
            deviceType = "PQ Bus";
            break;
        case 3:
            deviceType = "Line";
            break;
        case 4:
            deviceType = "Battery";
            break;
        case 5:
            deviceType = "Transformator";
            break;
    }
    gtk_label_set_text(GTK_LABEL(lblDevice), deviceType);
    gtk_label_set_xalign(GTK_LABEL(lblDevice), 0);
    gtk_grid_attach(GTK_GRID(grid), lblDevice, 0, 11, 1, 1);
    
    if(idx != 3)
    {
        GtkGesture *pressElement;
        pressElement = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(pressElement), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller (fixed, GTK_EVENT_CONTROLLER(pressElement));
        g_signal_connect (pressElement, "pressed", G_CALLBACK (pressed), fixed);
    }
    else
    {
        GtkGesture *pressLine;
        pressLine = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(pressLine), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller (fixed, GTK_EVENT_CONTROLLER(pressLine));
        g_signal_connect (pressLine, "pressed", G_CALLBACK (pressedLine), fixed);
    }
    //gtk_window_destroy(GTK_WINDOW(grid));
}

static void addElement(GtkWidget* btn)
{

    GtkWidget *scrolled, *flowbox;
    GtkWidget* gridPopover = gtk_grid_new();
    scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    flowbox = gtk_flow_box_new();
    gtk_widget_set_valign (flowbox, GTK_ALIGN_START);
    gtk_flow_box_set_max_children_per_line (GTK_FLOW_BOX (flowbox), 6);
    //gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (flowbox), GTK_SELECTION_SINGLE);
    gtk_grid_set_column_homogeneous (GTK_GRID(gridPopover),TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), flowbox);
    GtkWidget *popover;
    popover = gtk_popover_new();
    gtk_widget_set_size_request(popover, 200, 140);
    
    lblDevice = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(gridPopover), scrolled, 0, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(gridPopover), gtk_label_new(""), 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(gridPopover), lblDevice, 0, 11, 1, 1);
    if(!i)
    {
        gtk_popover_set_child(GTK_POPOVER(popover), gtk_label_new("You need to select a color first!"));
        gtk_widget_set_size_request(popover, 200, 150);
        gtk_widget_set_valign(popover, GTK_ALIGN_END);
        gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);
        gtk_menu_button_set_popover (GTK_MENU_BUTTON (btn), popover);
    }
    
    else
    {
        gtk_popover_set_child(GTK_POPOVER(popover), gridPopover);
        gtk_widget_set_valign(popover, GTK_ALIGN_END);
        gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);
        gtk_menu_button_set_popover (GTK_MENU_BUTTON (btn), popover);
        for(int j = 1; j<= 6; j++)
            gtk_flow_box_insert (GTK_FLOW_BOX (flowbox), colorElement(colors[i],j), -1);
        g_signal_connect (flowbox, "child-activated", G_CALLBACK (activateDevice), gridPopover);
    }
}

static void
exportToLatex(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    exportToLatex(systemModel);
}

static void
exportToHTML(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    exportToHTML(systemModel);
}

//static void
//exportToText(GSimpleAction *action, GVariant *parameter, gpointer user_data)
//{
//    exportToTxt("test.txt", systemModel, pngPathName);
//}

static void exportMenu(GtkWidget *btn)
{
    static const GActionEntry actions[]
    {
        {"latex", exportToLatex, NULL, NULL, NULL, {0, 0, 0}},
        {"html", exportToHTML, NULL, NULL, NULL, {0, 0, 0}},
        {"txt", exportToText, NULL, NULL, NULL, {0, 0, 0}},
        {"import", importFromText, NULL, NULL, NULL, {0, 0, 0}}
    };
    g_action_map_add_action_entries(G_ACTION_MAP(app), actions, G_N_ELEMENTS(actions), app);
    
    GMenu *menubar = g_menu_new ();
    
    GMenuItem *menu_item_Export = g_menu_item_new ("Export", NULL);
    GMenu *menuExport = g_menu_new ();

    GMenuItem *menu_item_latex = g_menu_item_new ("Latex (.tex)", "app.latex");
    g_menu_append_item (menuExport, menu_item_latex);
    g_object_unref (menu_item_latex);
    
    GMenuItem *menu_item_html = g_menu_item_new ("HTML (.html)", "app.html");
    g_menu_append_item (menuExport, menu_item_html);
    g_object_unref (menu_item_html);
    
    GMenuItem *menu_item_txt = g_menu_item_new ("Text (.txt)", "app.txt");
    g_menu_append_item (menuExport, menu_item_txt);
    g_object_unref (menu_item_txt);
    
    g_menu_item_set_submenu (menu_item_Export, G_MENU_MODEL (menuExport));
    g_menu_append_item(menubar, menu_item_Export);
    g_object_unref (menu_item_Export);
    
    GMenuItem *menu_item_Import = g_menu_item_new ("Import", "app.import");
    g_menu_append_item(menubar, menu_item_Import);
    g_object_unref (menu_item_Import);

    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(btn), G_MENU_MODEL (menubar));
}
int kPV, kSlack, kPQ, kLine, kBattery, kTrans;

static void setTreeViewRowLine(GtkWidget *entry, gpointer parent)
{
    GtkWidget *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt;
    
    int startBus, endBus;
    double real, imag, shunt;

    entryStart = gtk_grid_get_child_at(GTK_GRID(gridLine), 1, 0);
    entryEnd = gtk_grid_get_child_at(GTK_GRID(gridLine), 3, 0);
    entryReal = gtk_grid_get_child_at(GTK_GRID(gridLine), 1, 1);
    entryImag = gtk_grid_get_child_at(GTK_GRID(gridLine), 3, 1);
    entryShunt = gtk_grid_get_child_at(GTK_GRID(gridLine), 5, 1);
    
    startBus = getEntryInt(entryStart);
    endBus = getEntryInt(entryEnd);
    real = getEntryDbl(entryReal);
    imag = getEntryDbl(entryImag);
    shunt = getEntryDbl(entryShunt);
    
    std::get<0>(lineValues[kLine]) = startBus;
    std::get<1>(lineValues[kLine]) = endBus;
    std::get<2>(lineValues[kLine]) = real;
    std::get<3>(lineValues[kLine]) = imag;
    std::get<4>(lineValues[kLine]) = shunt;

    closeTab();
    lineTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridLine, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}

static void setTreeViewRowTrans(GtkWidget *entry, gpointer parent)
{
    GtkWidget *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt, *entryShuntReal;
    
    int startBus, endBus;
    double real, imag, shunt, shuntReal;

    entryStart = gtk_grid_get_child_at(GTK_GRID(gridTrans), 1, 0);
    entryEnd = gtk_grid_get_child_at(GTK_GRID(gridTrans), 3, 0);
    entryReal = gtk_grid_get_child_at(GTK_GRID(gridTrans), 1, 1);
    entryImag = gtk_grid_get_child_at(GTK_GRID(gridTrans), 3, 1);
    entryShuntReal = gtk_grid_get_child_at(GTK_GRID(gridTrans), 1, 2);
    entryShunt = gtk_grid_get_child_at(GTK_GRID(gridTrans), 3, 2);
    
    startBus = getEntryInt(entryStart);
    endBus = getEntryInt(entryEnd);
    real = getEntryDbl(entryReal);
    imag = getEntryDbl(entryImag);
    shuntReal = getEntryDbl(entryShuntReal);
    shunt = getEntryDbl(entryShunt);
    
    std::get<0>(transValues[kTrans]) = startBus;
    std::get<1>(transValues[kTrans]) = endBus;
    std::get<2>(transValues[kTrans]) = real;
    std::get<3>(transValues[kTrans]) = imag;
    std::get<4>(transValues[kTrans]) = shuntReal;
    std::get<5>(transValues[kTrans])= shunt;

    closeTab();
    transTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridTrans, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}


static void setTreeViewRowPQ(GtkWidget *entry, gpointer parent)
{
    int id;
    double active, reactive;

    GtkWidget *entryId, *entryAct, *entryReact;
    entryId = gtk_grid_get_child_at(GTK_GRID(gridPQ), 1, 0);
    entryAct = gtk_grid_get_child_at(GTK_GRID(gridPQ), 1, 1);
    entryReact = gtk_grid_get_child_at(GTK_GRID(gridPQ), 3, 1);
    id = getEntryInt(entryId);
    active = getEntryDbl(entryAct);
    reactive = getEntryDbl(entryReact);
    
    std::get<0>(pqValues[kPQ]) = id;
    std::get<1>(pqValues[kPQ]) = active;
    std::get<2>(pqValues[kPQ]) = reactive;
    closeTab();
    pqTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridPQ, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}

static void comboBoxSelChange(GtkWidget *comboBox, gpointer parent)
{
    const char* config = NULL;
    int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));
    switch(selected)
    {
        case 0:
            config = "Delta";
            break;
        case 1:
            config = "Grounded Star";
            break;
        case 2:
            config = "Star";
            break;
    }
    
    int id;
    double capacity;
    
    GtkWidget *entryId, *entryCapacity;
    entryId = gtk_grid_get_child_at(GTK_GRID(gridBattery), 1, 0);
    entryCapacity = gtk_grid_get_child_at(GTK_GRID(gridBattery), 3, 0);
    id = getEntryInt(entryId);
    capacity = getEntryDbl(entryCapacity);
    
    std::get<0>(batteryValues[kBattery]) = id;
    std::get<1>(batteryValues[kBattery]) = capacity;
    std::get<2>(batteryValues[kBattery]) = config;
    closeTab();
    batteryTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridBattery, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
    
}

static void setTreeViewRowBattery(GtkWidget *entry, gpointer parent)
{
    int id;
    double capacity;
    
    GtkWidget *entryId, *entryCapacity;
    entryId = gtk_grid_get_child_at(GTK_GRID(gridBattery), 1, 0);
    entryCapacity = gtk_grid_get_child_at(GTK_GRID(gridBattery), 3, 0);
//    comboBox = gtk_grid_get_child_at(GTK_GRID(gridBattery), 1, 1);
    
    id = getEntryInt(entryId);
    capacity = getEntryDbl(entryCapacity);
    
    std::get<0>(batteryValues[kBattery]) = id;
    std::get<1>(batteryValues[kBattery]) = capacity;
    closeTab();
    batteryTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridBattery, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}


static void setTreeViewRowSlack(GtkWidget *entry, gpointer parent)
{
    int id;
    double magnitude, phase;

    GtkWidget *entryId, *entryMag, *entryPhase;
    entryId = gtk_grid_get_child_at(GTK_GRID(gridSlack), 1, 0);
    entryMag = gtk_grid_get_child_at(GTK_GRID(gridSlack), 1, 1);
    entryPhase = gtk_grid_get_child_at(GTK_GRID(gridSlack), 3, 1);
    id = getEntryInt(entryId);
    magnitude = getEntryDbl(entryMag);
    phase = getEntryDbl(entryPhase);
    
    std::get<0>(slackValues[kSlack]) = id;
    std::get<1>(slackValues[kSlack]) = magnitude;
    std::get<2>(slackValues[kSlack]) = phase;
    closeTab();
    slackTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridSlack, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}

static void setTreeViewRowPV(GtkWidget *entry, gpointer parent)
{
    int id;
    double magnitude, active;

    GtkWidget *entryId, *entryMag, *entryActive;
    entryId = gtk_grid_get_child_at(GTK_GRID(gridPV), 1, 0);
    entryMag = gtk_grid_get_child_at(GTK_GRID(gridPV), 1, 1);
    entryActive = gtk_grid_get_child_at(GTK_GRID(gridPV), 3, 1);
    id = getEntryInt(entryId);
    magnitude = getEntryDbl(entryMag);
    active = getEntryDbl(entryActive);
    
    std::get<0>(pvValues[kPV]) = id;
    std::get<1>(pvValues[kPV]) = magnitude;
    std::get<2>(pvValues[kPV]) = active;
    closeTab();
    pvTable(NULL, NULL, NULL);
    gtk_notebook_reorder_child(GTK_NOTEBOOK(tab), gridPV, tabNumber);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(tab), tabNumber);
}

static void on_tree_view_row_activatedBattery(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryId, *entryCapacity, *comboBox;
    entryId = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryCapacity = gtk_grid_get_child_at(GTK_GRID(grid), 3, 0);
    comboBox = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    
    int id;
    double capacity;
    const char* config, *color;
    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, BUS_ID, &id,
                                                    CAPACITY, &capacity,
                                                    CONFIGURATION, &config,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryId, id);
    setEntryDbl(entryCapacity, capacity);
    int selectedConfig;
    if(!strcmp(config, "Delta"))
        selectedConfig = 0;
    else if (!strcmp(config, "Grounded Star"))
        selectedConfig = 1;
    else
        selectedConfig = 2;
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), selectedConfig);
    
    for(int i = 0; i < batteryValues.size(); i++)
    {
        const char* c_config = std::get<2>(batteryValues[i]).c_str();
        if(std::get<0>(batteryValues[i]) == id && std::get<1>(batteryValues[i]) == capacity && !(std::strcmp(c_config,config)))
        {
            kBattery = i;
            break;
        }
    }
    g_signal_connect (entryId, "activate", G_CALLBACK (setTreeViewRowBattery), NULL);
    g_signal_connect (entryCapacity, "activate", G_CALLBACK (setTreeViewRowBattery), NULL);
    g_signal_connect (comboBox, "changed", G_CALLBACK (comboBoxSelChange), NULL);
}


static void on_tree_view_row_activatedLine(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt;
    
    int startBus, endBus;
    double real, imag, shunt;
    const char* color;

    entryStart = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryEnd = gtk_grid_get_child_at(GTK_GRID(grid), 3, 0);
    entryReal = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    entryImag = gtk_grid_get_child_at(GTK_GRID(grid), 3, 1);
    entryShunt = gtk_grid_get_child_at(GTK_GRID(grid), 5, 1);

    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, START_BUS, &startBus,
                                                    END_BUS, &endBus,
                                                    REAL_IMPEDANCE, &real,
                                                    IMAG_IMPEDANCE, &imag,
                                                    SHUNT, &shunt,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryStart, startBus);
    setEntryInt(entryEnd, endBus);
    setEntryDbl(entryReal, real);
    setEntryDbl(entryImag, imag);
    setEntryDbl(entryShunt, shunt);
    
    for(int i = 0; i < lineValues.size(); i++)
    {
        if(std::get<0>(lineValues[i]) == startBus && std::get<1>(lineValues[i]) == endBus && std::get<2>(lineValues[i]) == real && std::get<3>(lineValues[i]) == imag && std::get<4>(lineValues[i]) == shunt)
        {
            kLine = i;
            break;
        }
    }
    g_signal_connect (entryStart, "activate", G_CALLBACK (setTreeViewRowLine), NULL);
    g_signal_connect (entryEnd, "activate", G_CALLBACK (setTreeViewRowLine), NULL);
    g_signal_connect (entryReal, "activate", G_CALLBACK (setTreeViewRowLine), NULL);
    g_signal_connect (entryImag, "activate", G_CALLBACK (setTreeViewRowLine), NULL);
    g_signal_connect (entryShunt, "activate", G_CALLBACK (setTreeViewRowLine), NULL);
}

static void on_tree_view_row_activatedTrans(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt, *entryShuntReal;
    
    int startBus, endBus;
    double real, imag, shunt, shuntReal;
    const char* color;

    entryStart = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryEnd = gtk_grid_get_child_at(GTK_GRID(grid), 3, 0);
    entryReal = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    entryImag = gtk_grid_get_child_at(GTK_GRID(grid), 3, 1);
    entryShuntReal = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
    entryShunt = gtk_grid_get_child_at(GTK_GRID(grid), 3, 2);

    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, START_BUS, &startBus,
                                                    END_BUS, &endBus,
                                                    REAL_IMPEDANCE, &real,
                                                    IMAG_IMPEDANCE, &imag,
                                                    SHUNT_REAL, &shuntReal,
                                                    SHUNT, &shunt,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryStart, startBus);
    setEntryInt(entryEnd, endBus);
    setEntryDbl(entryReal, real);
    setEntryDbl(entryImag, imag);
    setEntryDbl(entryShuntReal, shuntReal);
    setEntryDbl(entryShunt, shunt);
    
    for(int i = 0; i < transValues.size(); i++)
    {
        if(std::get<0>(transValues[i]) == startBus && std::get<1>(transValues[i]) == endBus && std::get<2>(transValues[i]) == real && std::get<3>(transValues[i]) == imag && std::get<4>(transValues[i]) == shuntReal && std::get<5>(transValues[i]) == shunt)
        {
            kTrans = i;
            break;
        }
    }
    g_signal_connect (entryStart, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
    g_signal_connect (entryEnd, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
    g_signal_connect (entryReal, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
    g_signal_connect (entryImag, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
    g_signal_connect (entryShuntReal, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
    g_signal_connect (entryShunt, "activate", G_CALLBACK (setTreeViewRowTrans), NULL);
}


static void on_tree_view_row_activatedPQ(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryId, *entryAct, *entryReact;
    entryId = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryAct = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    entryReact = gtk_grid_get_child_at(GTK_GRID(grid), 3, 1);
    
    int id;
    double active, reactive;
    const char* color;
    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, BUS_ID, &id,
                                                    ACTIVE_POWER, &active,
                                                    REACTIVE_POWER, &reactive,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryId, id);
    setEntryDbl(entryAct, active);
    setEntryDbl(entryReact, reactive);
    
    for(int i = 0; i < pqValues.size(); i++)
    {
        if(std::get<0>(pqValues[i]) == id && std::get<1>(pqValues[i]) == active && std::get<2>(pqValues[i]) == reactive)
        {
            kPQ = i;
            break;
            
        }
    }
    g_signal_connect (entryId, "activate", G_CALLBACK (setTreeViewRowPQ), NULL);
    g_signal_connect (entryAct, "activate", G_CALLBACK (setTreeViewRowPQ), NULL);
    g_signal_connect (entryReact, "activate", G_CALLBACK (setTreeViewRowPQ), NULL);
}

static void on_tree_view_row_activatedSlack(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryId, *entryMag, *entryPhase;
    entryId = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryMag = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    entryPhase = gtk_grid_get_child_at(GTK_GRID(grid), 3, 1);
    
    int id;
    double magnitude, phase;
    const char* color;
    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, BUS_ID, &id,
                                                    VOLTAGE_MAGNITUDE, &magnitude,
                                                    VOLTAGE_PHASE, &phase,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryId, id);
    setEntryDbl(entryMag, magnitude);
    setEntryDbl(entryPhase, phase);
    
    for(int i = 0; i < slackValues.size(); i++)
    {
        if(std::get<0>(slackValues[i]) == id && std::get<1>(slackValues[i]) == magnitude && std::get<2>(slackValues[i]) == phase)
        {
            kSlack = i;
            break;
        }
    }
    g_signal_connect (entryId, "activate", G_CALLBACK (setTreeViewRowSlack), NULL);
    g_signal_connect (entryMag, "activate", G_CALLBACK (setTreeViewRowSlack), NULL);
    g_signal_connect (entryPhase, "activate", G_CALLBACK (setTreeViewRowSlack), NULL);
}

static void on_tree_view_row_activatedPV(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,                                           GtkWidget* grid)
{
    GtkWidget *entryId, *entryMag, *entryActive;
    entryId = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    entryMag = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    entryActive = gtk_grid_get_child_at(GTK_GRID(grid), 3, 1);
    
    int id;
    double magnitude, active;
    const char* color;
    GtkTreeIter iter;
    GtkListStore* store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, BUS_ID, &id,
                                                    VOLTAGE_MAGNITUDE, &magnitude,
                                                    ACTIVE_POWER, &active,
                                                    COLOR, &color,
                                                      -1);
    setEntryInt(entryId, id);
    setEntryDbl(entryMag, magnitude);
    setEntryDbl(entryActive, active);
    
    for(int i = 0; i < pvValues.size(); i++)
    {
        if(std::get<0>(pvValues[i]) == id && std::get<1>(pvValues[i]) == magnitude && std::get<2>(pvValues[i]) == active)
        {
            kPV = i;
            break;
        }
    }
    g_signal_connect (entryId, "activate", G_CALLBACK (setTreeViewRowPV), NULL);
    g_signal_connect (entryMag, "activate", G_CALLBACK (setTreeViewRowPV), NULL);
    g_signal_connect (entryActive, "activate", G_CALLBACK (setTreeViewRowPV), NULL);
}

static void slackTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{

    gridSlack = gtk_grid_new();
    gtk_grid_set_column_homogeneous (GTK_GRID(gridSlack),TRUE);

    GtkWidget *busId, *voltageMagnitude, *voltagePhase, *entryId, *entryMag, *entryPhase;
    busId = gtk_label_new("Bus ID: ");
    voltageMagnitude = gtk_label_new("Voltage magnitude: ");
    voltagePhase = gtk_label_new("Voltage phase: ");
    
    //entry polja
    entryId = gtk_entry_new();
    entryMag = gtk_entry_new();
    entryPhase = gtk_entry_new();

    gtk_grid_set_column_homogeneous(GTK_GRID(gridSlack), TRUE);
    
    gtk_grid_attach (GTK_GRID (gridSlack), busId, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (gridSlack), entryId, 1, 0, 1, 1);
    
    gtk_grid_attach (GTK_GRID (gridSlack), voltageMagnitude, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridSlack), entryMag, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridSlack), voltagePhase, 2, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridSlack), entryPhase, 3, 1, 1, 1);
        
    //Table View
    GtkWidget *sw;
    GtkWidget *remove, *add, *entry, *removeAll;
    GtkWidget *vbox, *hbox;
    slackList = gtk_tree_view_new();
    initSlackList(slackList, slackValues);
    
    GtkTreeSelection *selection;
    sw = gtk_scrolled_window_new();
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(slackList), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), slackList);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(slackList), TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(slackList), GTK_TREE_VIEW_GRID_LINES_BOTH);
    
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(vbox), sw);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    add = gtk_button_new_with_label("Add");
    remove = gtk_button_new_with_label("Remove");
    removeAll = gtk_button_new_with_label("Remove All");
    entry = gtk_entry_new();

    gtk_box_append(GTK_BOX(hbox), add);
    gtk_box_append(GTK_BOX(hbox), entry);
    gtk_box_append(GTK_BOX(hbox), remove);
    gtk_box_append(GTK_BOX(hbox), removeAll);
    gtk_box_append(GTK_BOX(vbox), hbox);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(slackList));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(slackList), TRUE);
//    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
//    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
    g_signal_connect(slackList, "row-activated", G_CALLBACK(on_tree_view_row_activatedSlack), gridSlack);
    gtk_widget_set_vexpand(vbox, TRUE);
    gtk_widget_set_hexpand(vbox, TRUE);
    gtk_grid_attach(GTK_GRID(gridSlack), vbox, 0, 4, 10, 10);
    
    
    GtkWidget *head, *image, *btn, *label = gtk_label_new ("Slack Editing View");
    head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append (GTK_BOX (head), label);
    btn = gtk_button_new();
    gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
    image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
    g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
    gtk_button_set_child(GTK_BUTTON(btn), image);
    gtk_box_append (GTK_BOX (head), btn);
    gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridSlack, head);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridSlack, TRUE);

}


static void pvTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    gridPV = gtk_grid_new();
    gtk_grid_set_column_homogeneous (GTK_GRID(gridPV),TRUE);

    GtkWidget *busId, *voltageMagnitude, *activePower, *entryId, *entryMag, *entryActive;
    busId = gtk_label_new("Bus ID: ");
    voltageMagnitude = gtk_label_new("Voltage magnitude: ");
    activePower = gtk_label_new("Active Power: ");
    
    //entry polja
    entryId = gtk_entry_new();
    entryMag = gtk_entry_new();
    entryActive = gtk_entry_new();

    gtk_grid_set_column_homogeneous(GTK_GRID(gridPV), TRUE);
    
    gtk_grid_attach (GTK_GRID (gridPV), busId, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (gridPV), entryId, 1, 0, 1, 1);
    
    gtk_grid_attach (GTK_GRID (gridPV), voltageMagnitude, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridPV), entryMag, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridPV), activePower, 2, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridPV), entryActive, 3, 1, 1, 1);
        
    
    //Table View
//    GtkWidget *sw;
    GtkWidget *remove, *add, *entry, *removeAll;
    pvList = gtk_tree_view_new();
    initPvList(pvList, pvValues);
    
    GtkTreeSelection *selection;
//    sw = gtk_scrolled_window_new();
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(pvList), TRUE);
//    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), pvList);
//
//    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pvList), TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(pvList), GTK_TREE_VIEW_GRID_LINES_BOTH);
    
    gtk_widget_set_vexpand(pvList, TRUE);
    gtk_widget_set_hexpand(pvList, TRUE);
    GtkWidget* vboxPV = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(vboxPV), pvList);

    GtkWidget* hboxPV = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    add = gtk_button_new_with_label("Add");
    remove = gtk_button_new_with_label("Remove");
    removeAll = gtk_button_new_with_label("Remove All");
    entry = gtk_entry_new();

    gtk_box_append(GTK_BOX(hboxPV), add);
    gtk_box_append(GTK_BOX(hboxPV), entry);
    gtk_box_append(GTK_BOX(hboxPV), remove);
    gtk_box_append(GTK_BOX(hboxPV), removeAll);
    gtk_box_append(GTK_BOX(vboxPV), hboxPV);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pvList));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(pvList), TRUE);
//    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
//    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
    g_signal_connect(pvList, "row-activated", G_CALLBACK(on_tree_view_row_activatedPV), gridPV);
    gtk_widget_set_vexpand(vboxPV, TRUE);
    gtk_widget_set_hexpand(vboxPV, TRUE);
    gtk_grid_attach(GTK_GRID(gridPV), vboxPV, 0, 4, 10, 10);
    
    
    GtkWidget *head, *image, *btn, *label = gtk_label_new ("PV Editing View");
    head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append (GTK_BOX (head), label);
    btn = gtk_button_new();
    gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
    image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
    g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
    gtk_button_set_child(GTK_BUTTON(btn), image);
    gtk_box_append (GTK_BOX (head), btn);
    gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridPV, head);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridPV, TRUE);
    tabNumber = gtk_notebook_get_current_page(GTK_NOTEBOOK(tab));
}
    
static void pqTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        gridPQ = gtk_grid_new();
        gtk_grid_set_column_homogeneous (GTK_GRID(gridPQ),TRUE);

        GtkWidget *busId, *activePower, *reactivePower, *entryId, *entryActive, *entryReactive;
        busId = gtk_label_new("Bus ID: ");
        activePower = gtk_label_new("Active Power: ");
        reactivePower = gtk_label_new("Reactive Power: ");
        
        //entry polja
        entryId = gtk_entry_new();
        entryActive = gtk_entry_new();
        entryReactive = gtk_entry_new();

        gtk_grid_set_column_homogeneous(GTK_GRID(gridPQ), TRUE);
        
        gtk_grid_attach (GTK_GRID (gridPQ), busId, 0, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridPQ), entryId, 1, 0, 1, 1);
        
        gtk_grid_attach (GTK_GRID (gridPQ), activePower, 0, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridPQ), entryActive, 1, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridPQ), reactivePower, 2, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridPQ), entryReactive, 3, 1, 1, 1);
            
        
        //Table View
        GtkWidget *sw;
        GtkWidget *remove, *add, *entry, *removeAll;
        GtkWidget *vbox, *hbox;
        pqList = gtk_tree_view_new();
        initPqList(pqList, pqValues);
        
        GtkTreeSelection *selection;
        sw = gtk_scrolled_window_new();
        gtk_tree_view_set_reorderable(GTK_TREE_VIEW(pqList), TRUE);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), pqList);

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pqList), TRUE);
        gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(pqList), GTK_TREE_VIEW_GRID_LINES_BOTH);
        
        gtk_widget_set_vexpand(sw, TRUE);
        gtk_widget_set_hexpand(sw, TRUE);
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_append(GTK_BOX(vbox), sw);

        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
        add = gtk_button_new_with_label("Add");
        remove = gtk_button_new_with_label("Remove");
        removeAll = gtk_button_new_with_label("Remove All");
        entry = gtk_entry_new();

        gtk_box_append(GTK_BOX(hbox), add);
        gtk_box_append(GTK_BOX(hbox), entry);
        gtk_box_append(GTK_BOX(hbox), remove);
        gtk_box_append(GTK_BOX(hbox), removeAll);
        gtk_box_append(GTK_BOX(vbox), hbox);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pqList));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
        
        gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(pqList), TRUE);
    //    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
    //    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
        g_signal_connect(pqList, "row-activated", G_CALLBACK(on_tree_view_row_activatedPQ), gridPQ);
        gtk_widget_set_vexpand(vbox, TRUE);
        gtk_widget_set_hexpand(vbox, TRUE);
        gtk_grid_attach(GTK_GRID(gridPQ), vbox, 0, 4, 10, 10);
        
        
        GtkWidget *head, *image, *btn, *label = gtk_label_new ("PQ Editing View");
        head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append (GTK_BOX (head), label);
        btn = gtk_button_new();
        gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
        image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
        g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
        gtk_button_set_child(GTK_BUTTON(btn), image);
        gtk_box_append (GTK_BOX (head), btn);
        gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridPQ, head);
        gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridPQ, TRUE);
        tabNumber = gtk_notebook_get_current_page(GTK_NOTEBOOK(tab));
    }

static void lineTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        gridLine = gtk_grid_new();
        gtk_grid_set_column_homogeneous (GTK_GRID(gridLine),TRUE);

       GtkWidget *startBus, *endBus, *real, *imag, *shunt, *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt;
        startBus = gtk_label_new("Start Bus: ");
        endBus = gtk_label_new("End Bus: ");
        real = gtk_label_new("Re(Z): ");
        imag = gtk_label_new("Im(Z): ");
        shunt = gtk_label_new("Im(Y_sh): ");
        //entry polja
        entryStart = gtk_entry_new();
        entryEnd = gtk_entry_new();
        entryReal = gtk_entry_new();
        entryImag = gtk_entry_new();
        entryShunt = gtk_entry_new();

        gtk_grid_set_column_homogeneous(GTK_GRID(gridLine), TRUE);
        
        gtk_grid_attach (GTK_GRID (gridLine), startBus, 0, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), entryStart, 1, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), endBus, 2, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), entryEnd, 3, 0, 1, 1);
        
        gtk_grid_attach (GTK_GRID (gridLine), real, 0, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), entryReal, 1, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), imag, 2, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), entryImag, 3, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), shunt, 4, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridLine), entryShunt, 5, 1, 1, 1);
        
        //Table View
        GtkWidget *sw;
        GtkWidget *remove, *add, *entry, *removeAll;
        GtkWidget *vbox, *hbox;
        lineList = gtk_tree_view_new();
        initLineList(lineList, lineValues);
        
        GtkTreeSelection *selection;
        sw = gtk_scrolled_window_new();
        gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lineList), TRUE);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), lineList);

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lineList), TRUE);
        gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(lineList), GTK_TREE_VIEW_GRID_LINES_BOTH);
        
        gtk_widget_set_vexpand(sw, TRUE);
        gtk_widget_set_hexpand(sw, TRUE);
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_append(GTK_BOX(vbox), sw);

        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
        add = gtk_button_new_with_label("Add");
        remove = gtk_button_new_with_label("Remove");
        removeAll = gtk_button_new_with_label("Remove All");
        entry = gtk_entry_new();

        gtk_box_append(GTK_BOX(hbox), add);
        gtk_box_append(GTK_BOX(hbox), entry);
        gtk_box_append(GTK_BOX(hbox), remove);
        gtk_box_append(GTK_BOX(hbox), removeAll);
        gtk_box_append(GTK_BOX(vbox), hbox);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lineList));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
        
        gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(lineList), TRUE);
    //    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
    //    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
        g_signal_connect(lineList, "row-activated", G_CALLBACK(on_tree_view_row_activatedLine), gridLine);
        gtk_widget_set_vexpand(vbox, TRUE);
        gtk_widget_set_hexpand(vbox, TRUE);
        gtk_grid_attach(GTK_GRID(gridLine), vbox, 0, 4, 10, 10);
        
        
        GtkWidget *head, *image, *btn, *label = gtk_label_new ("Line Editing View");
        head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append (GTK_BOX (head), label);
        btn = gtk_button_new();
        gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
        image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
        gtk_button_set_child(GTK_BUTTON(btn), image);
        g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
        gtk_box_append (GTK_BOX (head), btn);
        gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridLine, head);
        gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridLine, TRUE);
}

static void batteryTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{

    gridBattery = gtk_grid_new();
    gtk_grid_set_column_homogeneous (GTK_GRID(gridBattery),TRUE);

    GtkWidget *busId, *capacity, *configuration, *entryId, *entryCapacity;
    busId = gtk_label_new("Connected Bus: ");
    capacity = gtk_label_new("Capacity: ");
    configuration = gtk_label_new("Configuration: ");
    
    //combo box for configuration
    GtkWidget* comboBox;
    comboBox = gtk_combo_box_text_new();

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox),"1", "Delta");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox),"2", "Grounded Star");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox),"3", "Star");

    gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);
        
    //entry polja
    entryId = gtk_entry_new();
    entryCapacity = gtk_entry_new();

    gtk_grid_set_column_homogeneous(GTK_GRID(gridBattery), TRUE);
    
    gtk_grid_attach (GTK_GRID (gridBattery), busId, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (gridBattery), entryId, 1, 0, 1, 1);
    
    gtk_grid_attach (GTK_GRID (gridBattery), capacity, 2, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (gridBattery), entryCapacity, 3, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (gridBattery), configuration, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (gridBattery), comboBox, 1, 1, 1, 1);
        
    //Table View
    GtkWidget *sw;
    GtkWidget *remove, *add, *entry, *removeAll;
    GtkWidget *vbox, *hbox;
    batteryList = gtk_tree_view_new();
    initBatteryList(batteryList, batteryValues);
    
    GtkTreeSelection *selection;
    sw = gtk_scrolled_window_new();
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(batteryList), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), batteryList);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(batteryList), TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(batteryList), GTK_TREE_VIEW_GRID_LINES_BOTH);
    
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(vbox), sw);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    add = gtk_button_new_with_label("Add");
    remove = gtk_button_new_with_label("Remove");
    removeAll = gtk_button_new_with_label("Remove All");
    entry = gtk_entry_new();

    gtk_box_append(GTK_BOX(hbox), add);
    gtk_box_append(GTK_BOX(hbox), entry);
    gtk_box_append(GTK_BOX(hbox), remove);
    gtk_box_append(GTK_BOX(hbox), removeAll);
    gtk_box_append(GTK_BOX(vbox), hbox);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(batteryList));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
    
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(batteryList), TRUE);
//    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
//    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
    g_signal_connect(batteryList, "row-activated", G_CALLBACK(on_tree_view_row_activatedBattery), gridBattery);
    gtk_widget_set_vexpand(vbox, TRUE);
    gtk_widget_set_hexpand(vbox, TRUE);
    gtk_grid_attach(GTK_GRID(gridBattery), vbox, 0, 4, 10, 10);
    
    
    GtkWidget *head, *image, *btn, *label = gtk_label_new ("Battery Editing View");
    head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append (GTK_BOX (head), label);
    btn = gtk_button_new();
    gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
    image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
    g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
    gtk_button_set_child(GTK_BUTTON(btn), image);
    gtk_box_append (GTK_BOX (head), btn);
    gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridBattery, head);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridBattery, TRUE);
}

static void transTable(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        gridTrans = gtk_grid_new();
        gtk_grid_set_column_homogeneous (GTK_GRID(gridTrans),TRUE);

       GtkWidget *startBus, *endBus, *real, *imag, *shunt, *shunt_real, *entryStart, *entryEnd, *entryReal, *entryImag, *entryShunt, *entryShuntReal;
        startBus = gtk_label_new("Start Bus: ");
        endBus = gtk_label_new("End Bus: ");
        real = gtk_label_new("Re(Z): ");
        imag = gtk_label_new("Im(Z): ");
        shunt_real = gtk_label_new("Re(Y_sh): ");
        shunt = gtk_label_new("Im(Y_sh): ");
        //entry polja
        entryStart = gtk_entry_new();
        entryEnd = gtk_entry_new();
        entryReal = gtk_entry_new();
        entryImag = gtk_entry_new();
        entryShuntReal = gtk_entry_new();
        entryShunt = gtk_entry_new();

        gtk_grid_set_column_homogeneous(GTK_GRID(gridTrans), TRUE);
        
        gtk_grid_attach (GTK_GRID (gridTrans), startBus, 0, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryStart, 1, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), endBus, 2, 0, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryEnd, 3, 0, 1, 1);
        
        gtk_grid_attach (GTK_GRID (gridTrans), real, 0, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryReal, 1, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), imag, 2, 1, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryImag, 3, 1, 1, 1);
    
        gtk_grid_attach (GTK_GRID (gridTrans), shunt_real, 0, 2, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryShuntReal, 1, 2, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), shunt, 2, 2, 1, 1);
        gtk_grid_attach (GTK_GRID (gridTrans), entryShunt, 3, 2, 1, 1);
        
        //Table View
        GtkWidget *sw;
        GtkWidget *remove, *add, *entry, *removeAll;
        GtkWidget *vbox, *hbox;
        transList = gtk_tree_view_new();
        initTransformatorList(transList, transValues);
        
        GtkTreeSelection *selection;
        sw = gtk_scrolled_window_new();
        gtk_tree_view_set_reorderable(GTK_TREE_VIEW(transList), TRUE);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), transList);

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(transList), TRUE);
        gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(transList), GTK_TREE_VIEW_GRID_LINES_BOTH);
        
        gtk_widget_set_vexpand(sw, TRUE);
        gtk_widget_set_hexpand(sw, TRUE);
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_append(GTK_BOX(vbox), sw);

        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
        add = gtk_button_new_with_label("Add");
        remove = gtk_button_new_with_label("Remove");
        removeAll = gtk_button_new_with_label("Remove All");
        entry = gtk_entry_new();

        gtk_box_append(GTK_BOX(hbox), add);
        gtk_box_append(GTK_BOX(hbox), entry);
        gtk_box_append(GTK_BOX(hbox), remove);
        gtk_box_append(GTK_BOX(hbox), removeAll);
        gtk_box_append(GTK_BOX(vbox), hbox);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(transList));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        //path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
        
        gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(transList), TRUE);
    //    g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(remove_item), selection);
    //    g_signal_connect(G_OBJECT(removeAll), "clicked", G_CALLBACK(remove_all), selection);
        g_signal_connect(transList, "row-activated", G_CALLBACK(on_tree_view_row_activatedTrans), gridTrans);
        gtk_widget_set_vexpand(vbox, TRUE);
        gtk_widget_set_hexpand(vbox, TRUE);
        gtk_grid_attach(GTK_GRID(gridTrans), vbox, 0, 4, 10, 10);
        
        GtkWidget *head, *image, *btn, *label = gtk_label_new ("Transformator Editing View");
        head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append (GTK_BOX (head), label);
        btn = gtk_button_new();
        gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);
        image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
        gtk_button_set_child(GTK_BUTTON(btn), image);
        g_signal_connect (btn, "clicked", G_CALLBACK (closeTab), NULL);
        gtk_box_append (GTK_BOX (head), btn);
        gtk_notebook_append_page (GTK_NOTEBOOK (tab), gridTrans, head);
        gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), gridTrans, TRUE);
}

static void editMenu(GtkWidget *btn)
{
    static const GActionEntry actions[]
    {
        {"slack", slackTable, NULL, NULL, NULL, {0, 0, 0}},
        {"pv", pvTable, NULL, NULL, NULL, {0, 0, 0}},
        {"pq", pqTable, NULL, NULL, NULL, {0, 0, 0}},
        {"line", lineTable, NULL, NULL, NULL, {0, 0, 0}},
        {"battery", batteryTable, NULL, NULL, NULL, {0, 0, 0}},
        {"transformator", transTable, NULL, NULL, NULL, {0, 0, 0}}
    };
    g_action_map_add_action_entries(G_ACTION_MAP(app), actions, G_N_ELEMENTS(actions), app);
    
    
    GMenu *menuEdit = g_menu_new ();

    GMenuItem *menu_item_slack = g_menu_item_new ("Slack Bus", "app.slack");
    g_menu_append_item (menuEdit, menu_item_slack);
    g_object_unref (menu_item_slack);
    
    GMenuItem *menu_item_pv = g_menu_item_new ("PV Bus", "app.pv");
    g_menu_append_item (menuEdit, menu_item_pv);
    g_object_unref (menu_item_pv);
    
    GMenuItem *menu_item_pq = g_menu_item_new ("PQ Bus", "app.pq");
    g_menu_append_item (menuEdit, menu_item_pq);
    g_object_unref (menu_item_pq);
    
    GMenuItem *menu_item_line = g_menu_item_new ("Line", "app.line");
    g_menu_append_item (menuEdit, menu_item_line);
    g_object_unref (menu_item_line);
    
    GMenuItem *menu_item_battery = g_menu_item_new ("Battery", "app.battery");
    g_menu_append_item (menuEdit, menu_item_battery);
    g_object_unref (menu_item_battery);
    
    GMenuItem *menu_item_transformator = g_menu_item_new ("Transformator", "app.transformator");
    g_menu_append_item (menuEdit, menu_item_transformator);
    g_object_unref (menu_item_transformator);
    
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(btn), G_MENU_MODEL (menuEdit));
}

static void
draw_color (GtkDrawingArea *drawingarea,
            cairo_t        *cr,
            int             width,
            int             height,
            gpointer        data)
{
  const char *color_name = (const char*)data;
  GdkRGBA rgba;

  if (gdk_rgba_parse (&rgba, color_name))
    {
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_paint (cr);
    }
}

static GtkWidget *
color_swatch_new (const char *color)
{
    GtkWidget *area;
    area = gtk_drawing_area_new ();

    gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (area), 1);
    gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (area), 20);

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), draw_color, (gpointer) color, NULL);

  return area;
}

GtkWidget *btnSelColor, *lblColor;

static void
colorSelected(GtkWidget *flowbox, GtkWidget *area, GtkWidget *grid)
{
    i = 0;
    GtkFlowBoxChild *ref;
    while(1)
    {
        ref = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flowbox), i);
        if(GTK_WIDGET(ref) == area)
            break;
        i++;
    }
    GtkDrawingArea *newArea;
    newArea = GTK_DRAWING_AREA(color_swatch_new (colors[i]));
    gtk_widget_set_size_request(GTK_WIDGET(newArea), 25, 20);
    gtk_button_set_child(GTK_BUTTON(btnSelColor), GTK_WIDGET(newArea));
    gtk_label_set_text((GtkLabel*)lblColor, colors[i]);
    gtk_label_set_xalign(GTK_LABEL(lblColor), 0);
    gtk_grid_attach(GTK_GRID(grid), lblColor, 0, 11, 1, 1);
    //gtk_window_destroy(GTK_WINDOW(grid));
    addElement(add);
}

GtkWidget* createColorPicker(void)
{
    GtkWidget *scrolled, *flowbox;
    int i;

    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    flowbox = gtk_flow_box_new ();
    gtk_widget_set_valign (flowbox, GTK_ALIGN_START);
    gtk_flow_box_set_max_children_per_line (GTK_FLOW_BOX (flowbox), 14);
    gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (flowbox), GTK_SELECTION_SINGLE);
    gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(flowbox), TRUE);

    GtkWidget *gridPopover;
    lblColor = gtk_label_new("");
    gridPopover = gtk_grid_new();
    gtk_grid_set_column_homogeneous (GTK_GRID(gridPopover),TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);

    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), flowbox);

    gtk_grid_attach(GTK_GRID(gridPopover), scrolled, 0, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(gridPopover), gtk_label_new(""), 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(gridPopover), lblColor, 0, 11, 1, 1);

    GtkWidget *popover;
    GtkWidget *customColorBtn, *button;
    customColorBtn = gtk_grid_new();
    btnSelColor = gtk_button_new();
    button = gtk_menu_button_new();
    //gtk_widget_set_size_request(button, 50, 50);
    gtk_grid_set_column_homogeneous (GTK_GRID(customColorBtn),TRUE);
    gtk_grid_attach(GTK_GRID(customColorBtn), btnSelColor, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(customColorBtn), button, 1, 0, 1, 1);
    popover = gtk_popover_new();
    gtk_widget_set_size_request(popover, 400, 150);

    //gtk_popover_set_has_arrow(popover, TRUE);

    gtk_popover_set_child(GTK_POPOVER(popover), gridPopover);
    gtk_widget_set_valign(popover, GTK_ALIGN_END);
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), popover);

    for (i = 0; colors[i]; i++)
        gtk_flow_box_insert (GTK_FLOW_BOX (flowbox), color_swatch_new (colors[i]), -1);
    g_signal_connect (flowbox, "child-activated", G_CALLBACK (colorSelected), gridPopover);

    return customColorBtn;

}

static void
clear_surface (void)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}

static void
resize_cb (GtkWidget *widget,
           int        width,
           int        height,
           gpointer   data)
{
  if (surface)
    {
      cairo_surface_destroy (surface);
      surface = NULL;
    }

  if (gtk_native_get_surface (gtk_widget_get_native (widget)))
    {
      surface = gdk_surface_create_similar_surface (gtk_native_get_surface (gtk_widget_get_native (widget)),
                                                    CAIRO_CONTENT_COLOR,
                                                    gtk_widget_get_width (widget),
                                                    gtk_widget_get_height (widget));
      clear_surface();
    }
}



static void
drawGrid (GtkWidget *widget)
{
    cairo_t *cr;
    cr = cairo_create (surface);
    cairo_set_source_rgb(cr, 0.55, 0.55, 0.55);
    cairo_set_line_width(cr, 0.5);
    for(int i = 0; i < 1590; i+=20)
    {
        cairo_move_to(cr, i, 0);
        cairo_line_to(cr, i, 800);
        cairo_stroke(cr);
    }
    
    for(int i = 0; i < 730; i+=20)
    {
        cairo_move_to(cr, 0, i);
        cairo_line_to(cr, 1590, i);
        cairo_stroke(cr);
    }
    
    cairo_destroy (cr);

    gtk_widget_queue_draw (widget);
}

int grid = 1;
static void
draw_cb (GtkDrawingArea *drawing_area,
         cairo_t        *cr,
         int             width,
         int             height,
         gpointer        data)
{
    if(grid)
    {
        drawGrid(area);
        grid = 0;
    }

  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);
}

int checkCircleInside(int xCenter, int yCenter, int x, int y, int radius)
{
    double val = (x - xCenter)^2 + (y - yCenter)^2;
    if(val < (radius^2))
        return 1;
    return 0;
}

static void switch_page (GtkButton *button, GtkNotebook *notebook)
{
    gint page = gtk_notebook_get_current_page(notebook);
    if (page == 0)
        gtk_notebook_set_current_page(notebook, 1);
    else
        gtk_notebook_set_current_page(notebook, 0);
}

void remove_item(GtkWidget *widget, gpointer selection)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter  iter;

    store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
      return;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter))
        gtk_list_store_remove(store, &iter);
}

void remove_all(GtkWidget *widget, gpointer selection)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter  iter;
    
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING);
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
        return;
    gtk_list_store_clear(store);
}

static void solveNR(GtkWidget* btn, GtkWidget* dialog)
{
    if(!importFlag)
    {
        buses.push_back(slackValues);
        buses.push_back(pvValues);
        buses.push_back(pqValues);
    }

    GdkDisplay *display;
    GtkCssProvider *cssProvider;

    GtkWidget *view;
    display = gtk_widget_get_display (GTK_WIDGET (dialog));
    cssProvider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (cssProvider, "textview { font-size: 14pt; }", -1);
    gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    
    view = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    gtk_text_buffer_create_tag(buffer, "gap",
            "pixels_above_lines", 30, NULL);
    gtk_text_buffer_create_tag(buffer, "lmarg",
            "left_margin", 5, NULL);
    gtk_text_buffer_create_tag(buffer, "blue_fg",
            "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(buffer, "bold",
            "weight", PANGO_WEIGHT_BOLD, NULL);
    
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
            "Buses:\n", -1, "bold", "lmarg",  NULL);
    
    gtk_window_destroy(GTK_WINDOW(dialog));
    int k = 0, l = 0, m = 0;
    systemModel.addBus(SystemModel::TypeOfBus::Slack);
    for(int i = 0; i < sortedBuses.size(); i++)
    {
        int type = std::get<6>(sortedBuses[i]);
        switch(type)
        {
            case 1:
                char output[50];
                snprintf(output, 50, "%d", std::get<0>(slackValues[k]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\tType: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, "Slack", -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tBus ID: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, output, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                char magnitude[50], phase[50];
                snprintf(magnitude, 50, "%lf", std::get<1>(slackValues[k]));
                snprintf(phase, 50, "%lf", std::get<2>(slackValues[k]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tVoltage magnitude: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, magnitude, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tVoltage phase: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, phase, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                                       
                systemModel.getBus(std::get<0>(slackValues[k])).setVoltageMagnitude(std::get<1>(slackValues[k]));
                systemModel.getBus(std::get<0>(slackValues[k])).setVoltagePhase(std::get<2>(slackValues[k]));
                k++;
                break;
                
            case 2:
                systemModel.addBus(SystemModel::TypeOfBus::PV);
                char output2[50];
                snprintf(output2, 50, "%d", std::get<0>(pvValues[l]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\tType: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, "PV", -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tBus ID: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, output2, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                char magnitude2[50], active[50];
                snprintf(magnitude2, 50, "%lf", std::get<1>(pvValues[l]));
                snprintf(active, 50, "%lf", std::get<2>(pvValues[l]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tVoltage magnitude: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, magnitude2, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tActive power: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, active, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                systemModel.getBus(std::get<0>(pvValues[l])).setVoltageMagnitude(std::get<1>(pvValues[l]));
                systemModel.getBus(std::get<0>(pvValues[l])).setActivePower(std::get<2>(pvValues[l]));
                l++;
                break;
                
            case 3:
                systemModel.addBus(SystemModel::TypeOfBus::PQ);
                char output3[50];
                snprintf(output3, 50, "%d", std::get<0>(pqValues[m]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\tType: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, "PQ", -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tBus ID: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, output3, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                char reactive[50], active3[50];
                snprintf(active3, 50, "%lf", std::get<1>(pvValues[m]));
                snprintf(reactive, 50, "%lf", std::get<2>(pvValues[m]));
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tActive power: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, active3, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                        "\t\tReactive power: ", -1, "blue_fg", "lmarg",  NULL);
                gtk_text_buffer_insert(buffer, &iter, reactive, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
                
                systemModel.getBus(std::get<0>(pqValues[m])).setActivePower(std::get<1>(pqValues[m]));
                systemModel.getBus(std::get<0>(pqValues[m])).setReactivePower(std::get<2>(pqValues[m]));
                m++;
                break;
        }
    }
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
            "Branches:\n", -1, "bold", "lmarg",  NULL);
    
    for(int i = 0; i < lineValues.size(); i++)
    {
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\tType: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, "Line", -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        char b1[10], b2[10];
        snprintf(b1, 10, "%d", std::get<0>(lineValues[i]));
        snprintf(b2, 10, "%d", std::get<1>(lineValues[i]));
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tFrom Bus: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, b1, -1);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                " to Bus: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, b2, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        char resistance[50], reactance[50], susceptance[50];
        snprintf(resistance, 50, "%lf", std::get<2>(lineValues[i]));
        snprintf(reactance, 50, "%lf", std::get<3>(lineValues[i]));
        snprintf(susceptance, 50, "%lf", std::get<4>(lineValues[i]));
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tSeries resistance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, resistance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tSeries reactance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, reactance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tShunt susceptance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, susceptance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        systemModel.addLine(std::get<0>(lineValues[i]), std::get<1>(lineValues[i]), std::get<2>(lineValues[i]),std::get<3>(lineValues[i]), std::get<4>(lineValues[i]));
    }
    
    for(int i = 0; i < transValues.size(); i++)
    {
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\tType: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, "Transformer", -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        char b1[10], b2[10];
        snprintf(b1, 10, "%d", std::get<0>(transValues[i]));
        snprintf(b2, 10, "%d", std::get<1>(transValues[i]));
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tFrom Bus: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, b1, -1);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                " to Bus: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, b2, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        char resistance[50], reactance[50], conductance[50], susceptance[50];
        snprintf(resistance, 50, "%lf", std::get<2>(transValues[i]));
        snprintf(reactance, 50, "%lf", std::get<3>(transValues[i]));
        snprintf(conductance, 50, "%lf", std::get<4>(transValues[i]));
        snprintf(susceptance, 50, "%lf", std::get<5>(transValues[i]));
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tSeries resistance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, resistance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tSeries reactance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, reactance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tShunt conductance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, conductance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tShunt susceptance: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, susceptance, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        systemModel.addTransformer(std::get<0>(transValues[i]), std::get<1>(transValues[i]), std::get<2>(transValues[i]), std::get<3>(transValues[i]), std::get<4>(transValues[i]),std::get<5>(transValues[i]));
    }
    if(batteryValues.size() > 0)
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
                "Capacitor banks:\n", -1, "bold", "lmarg",  NULL);
    
    for(int i = 0; i < batteryValues.size(); i++)
    {
        char id[10];
        snprintf(id, 10, "%d", std::get<0>(batteryValues[i]));
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\tAt Bus: ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(buffer, &iter, id, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        const char* config = std::get<2>(batteryValues[i]).c_str();;
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tLoad Configuration : ", -1, "blue_fg", "lmarg",  NULL);
        if(!strcmp(config, "Delta"))
        {
            gtk_text_buffer_insert(buffer, &iter, config, -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            systemModel.addCapacitorBank(std::get<0>(batteryValues[i]), std::get<1>(batteryValues[i]), SystemModel::ThreePhaseLoadConfigurationsType::Delta);
        }
        else if(!strcmp(config, "Grounded Star"))
        {
            gtk_text_buffer_insert(buffer, &iter, config, -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            systemModel.addCapacitorBank(std::get<0>(batteryValues[i]), std::get<1>(batteryValues[i]), SystemModel::ThreePhaseLoadConfigurationsType::GroundedStar);
        }
        else
        {
            gtk_text_buffer_insert(buffer, &iter, config, -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            systemModel.addCapacitorBank(std::get<0>(batteryValues[i]), std::get<1>(batteryValues[i]), SystemModel::ThreePhaseLoadConfigurationsType::Star);
        }
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                "\t\tCapacitance : ", -1, "blue_fg", "lmarg",  NULL);
        char cap[50];
        snprintf(cap, 50, "%lf", std::get<1>(batteryValues[i]));
        gtk_text_buffer_insert(buffer, &iter, cap, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        
    }
    GtkWidget *sw;
    sw = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), view);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    gtk_widget_set_vexpand(sw, TRUE);
    gtk_widget_set_hexpand(sw, TRUE);
    std::cout << systemModel << std::endl;
    GtkWidget *head, *image, *btn1, *label = gtk_label_new ("Results");
    head = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append (GTK_BOX (head), label);
    btn1 = gtk_button_new();
    gtk_button_set_has_frame (GTK_BUTTON (btn1), FALSE);
    image = gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/close.png");
    g_signal_connect (btn1, "clicked", G_CALLBACK (closeTab), NULL);
    gtk_button_set_child(GTK_BUTTON(btn1), image);
    gtk_box_append (GTK_BOX (head), btn1);
    gtk_notebook_append_page (GTK_NOTEBOOK (tab), sw, head);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tab), sw, TRUE);
    
    std::vector<double> x0, x;
    int size = 0;
    for(int i = 0; i < 3; i++)
        size += buses[i].size();
    for(int i = 0; i < 2*size; i++)
        x0.push_back(1);

    
    double err;
    int maxNumberOfIter = 50, iterx;
    newtonRaphson(systemModel, maxNumberOfIter, eps, x0, x, err, iterx);

    GtkTextBuffer *bufferNR;
    GtkWidget* viewNR;

    viewNR = gtk_text_view_new();
    bufferNR = gtk_text_view_get_buffer(GTK_TEXT_VIEW(viewNR));

    gtk_text_buffer_create_tag(bufferNR, "blue_fg",
            "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(bufferNR, "lmarg",
            "left_margin", 5, NULL);

    gtk_text_buffer_get_iter_at_offset(bufferNR, &iter, 0);
    gtk_text_buffer_insert(bufferNR, &iter, "\n", -1);

    if (eps >= err)
    {
        char iteration[50], error[50];
        snprintf(iteration, 50, "%d", iterx);
        sprintf(error, "%e", err);

        gtk_text_buffer_insert_with_tags_by_name(bufferNR, &iter,
                " Algorithm converges in ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(bufferNR, &iter, iteration, -1);
        gtk_text_buffer_insert_with_tags_by_name(bufferNR, &iter,
                " iterations,\n with error ", -1, "blue_fg", "lmarg",  NULL);
        gtk_text_buffer_insert(bufferNR, &iter, error, -1);
    }
    if (iterx == maxNumberOfIter)
        gtk_text_buffer_insert_with_tags_by_name(bufferNR, &iter,
                " Maximum number of iterations reached.", -1, "blue_fg", "lmarg",  NULL);

    GtkWidget *dialog1, *content_area;
    gtk_widget_set_vexpand(viewNR, TRUE);
    dialog1 = gtk_dialog_new();
    gtk_dialog_add_buttons(GTK_DIALOG(dialog1), "Close", 1, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog1));
    g_signal_connect_swapped(dialog1, "response", G_CALLBACK(gtk_window_destroy), dialog1);
    gtk_box_append(GTK_BOX(content_area), viewNR);
    gtk_window_set_default_size(GTK_WINDOW(dialog1), 300, 100);
    gtk_window_set_title(GTK_WINDOW(dialog1), "Newton Raphson Result");
    gtk_widget_show(dialog1);

    
}

static void messageDialog(GtkWidget* button)
{
    GtkWidget *dialog, *content_area, *hbox1, *hbox2, *labelWS1, *labelWS2, *btnOk, *btnCancel, *hlabel;
    
    btnOk = gtk_button_new_with_label("Ok");
    btnCancel = gtk_button_new_with_label("Cancel");
    
    GtkImage* image;
    image = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/confirm.png"));
    gtk_image_set_icon_size(image, GTK_ICON_SIZE_NORMAL);
    
    dialog = gtk_dialog_new();
//    gtk_dialog_add_buttons(GTK_DIALOG(dialog), "Ok", 1, "Cancel", 2, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);
    hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    //gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
    labelWS1 = gtk_label_new("   ");
    labelWS2 = gtk_label_new("   ");
    hlabel = gtk_label_new("   ");
    gtk_widget_set_hexpand(hlabel, TRUE);
    
    gtk_box_append(GTK_BOX(content_area), gtk_label_new("   "));
    gtk_box_append(GTK_BOX(hbox1), labelWS1);
    gtk_box_append(GTK_BOX(hbox1), GTK_WIDGET(image));
    gtk_box_append(GTK_BOX(hbox1), gtk_label_new("Are you sure you want to continue?"));
    gtk_box_append(GTK_BOX(hbox1), labelWS2);
    gtk_box_append(GTK_BOX(content_area), hbox1);
    gtk_box_append(GTK_BOX(content_area), gtk_label_new("   "));
    gtk_box_append(GTK_BOX(hbox2), hlabel);
    gtk_box_append(GTK_BOX(hbox2), btnOk);
    gtk_box_append(GTK_BOX(hbox2), btnCancel);
    gtk_box_append(GTK_BOX(content_area), hbox2);
    
    g_signal_connect(btnOk, "clicked", G_CALLBACK(solveNR), dialog);
    g_signal_connect(btnCancel, "clicked", G_CALLBACK(gtk_window_destroy), dialog);

    gtk_window_set_title(GTK_WINDOW(dialog), "Message Dialog");
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    
    gtk_widget_show(dialog);
    
}
static void createToolBar(GtkWidget* boxToolBar)
{
    GtkImage* imagePlus;
    imagePlus = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/sign-add-icon.png"));
    gtk_image_set_icon_size(imagePlus, GTK_ICON_SIZE_LARGE);

    GtkImage* imageRemove;
    imageRemove = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/remove.png"));
    gtk_image_set_icon_size(imageRemove, GTK_ICON_SIZE_LARGE);
    
    GtkImage* imageFile;
    imageFile = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/export.png"));
    gtk_image_set_icon_size(imageFile, GTK_ICON_SIZE_LARGE);
    
    GtkImage* imageSolve;
    imageSolve = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/solve.png"));
    gtk_image_set_icon_size(imageSolve, GTK_ICON_SIZE_LARGE);
    
    GtkImage* imageEdit;
    imageEdit = GTK_IMAGE(gtk_image_new_from_file("/Users/dzeni/Documents/gtk/_NRGui/Icons/edit.png"));
    gtk_image_set_icon_size(imageEdit, GTK_ICON_SIZE_LARGE);

    GtkWidget* btnRemove, *btnFile, *btnEdit, * lblEmpty;

    
    //lblHeader = gtk_label_new("     Test GUI");
    //gtk_widget_set_size_request(lblHeader, 100, 100);
    lblEmpty = gtk_label_new(" ");
    gtk_widget_set_hexpand(lblEmpty, TRUE);
    add = gtk_menu_button_new();
    btnRemove = gtk_button_new();
    btnFile = gtk_menu_button_new();
    btnSolve = gtk_button_new();
    btnEdit = gtk_menu_button_new();

    GtkWidget* boxAdd, * boxRemove, *boxFile, *boxEdit, *boxSolve;
    boxAdd = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    boxRemove = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    boxFile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    boxSolve = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    boxEdit = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_box_append(GTK_BOX(boxAdd), GTK_WIDGET(imagePlus));
    gtk_box_append(GTK_BOX(boxAdd), gtk_label_new("Add"));

    gtk_box_append(GTK_BOX(boxRemove), GTK_WIDGET(imageRemove));
    gtk_box_append(GTK_BOX(boxRemove), gtk_label_new("Remove"));
    
    gtk_box_append(GTK_BOX(boxFile), GTK_WIDGET(imageFile));
    gtk_box_append(GTK_BOX(boxFile), gtk_label_new("File"));
    
    gtk_box_append(GTK_BOX(boxSolve), GTK_WIDGET(imageSolve));
    gtk_box_append(GTK_BOX(boxSolve), gtk_label_new("Solve"));
    
    gtk_box_append(GTK_BOX(boxEdit), GTK_WIDGET(imageEdit));
    gtk_box_append(GTK_BOX(boxEdit), gtk_label_new("Edit"));

    gtk_menu_button_set_child(GTK_MENU_BUTTON(add), boxAdd);
    gtk_button_set_child(GTK_BUTTON(btnRemove), boxRemove);
    gtk_menu_button_set_child(GTK_MENU_BUTTON(btnFile), boxFile);
    gtk_button_set_child(GTK_BUTTON(btnSolve), boxSolve);
    gtk_menu_button_set_child(GTK_MENU_BUTTON(btnEdit), boxEdit);
    
    //Color
    GtkWidget *colorPicker;
    colorPicker = createColorPicker();
    GtkWidget* colorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    //gtk_box_set_homogeneous(GTK_BOX(colorBox), TRUE);
    gtk_box_append(GTK_BOX(colorBox), colorPicker);
    gtk_box_append(GTK_BOX(colorBox), gtk_label_new("Color"));

    gtk_header_bar_pack_start(GTK_HEADER_BAR(boxToolBar), lblEmpty);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(boxToolBar), add);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(boxToolBar), btnRemove);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(boxToolBar), colorBox);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(boxToolBar), btnEdit);

    gtk_header_bar_pack_end(GTK_HEADER_BAR(boxToolBar), btnSolve);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(boxToolBar), btnFile);

    addElement(add);
    exportMenu(btnFile);
    editMenu(btnEdit);
    g_signal_connect (btnSolve, "clicked", G_CALLBACK (messageDialog), NULL);
}

static void createMainGrid(GtkWidget* mainGrid)
{

    GtkWidget* areaFrame = gtk_frame_new("Scheme");
    area = gtk_drawing_area_new();
    GtkWidget* boxArea = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    fixed = gtk_fixed_new();
    gtk_widget_set_hexpand(area, TRUE);
    gtk_widget_set_vexpand(area, TRUE);
    gtk_fixed_put(GTK_FIXED(fixed), area, 0, 0);
    gtk_frame_set_child(GTK_FRAME(areaFrame), fixed);
    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), draw_cb, NULL, NULL);
    
    g_signal_connect_after (area, "resize", G_CALLBACK (resize_cb), NULL);
    //gtk_widget_set_vexpand(area, TRUE);
    //gtk_widget_set_hexpand(area, TRUE);
    gtk_widget_set_size_request(area, 1590, 680);
    //drawLine(area);
    
    gtk_box_append(GTK_BOX(boxArea), areaFrame);
    
    gtk_box_append(GTK_BOX(boxArea), gtk_label_new("After selecting a device type and entering a value, click to place the device"));
    
    
    
    gtk_grid_attach(GTK_GRID(mainGrid), boxArea, 1, 0, 1, 1);
}

GtkCssProvider* cssProvider;
GtkStyleContext* cssStyleContext;
GdkDisplay *display;

static void on_activate (GtkApplication *app)
{
    win = gtk_application_window_new (GTK_APPLICATION (app));
    display = gtk_widget_get_display (GTK_WIDGET (win));
    cssProvider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (cssProvider, "entry { min-height: 15px;}", -1);
    gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_window_set_title (GTK_WINDOW (win), "PowerFlow Solver");
    gtk_window_set_default_size (GTK_WINDOW (win), 200, 500);
    
    GtkWidget* toolBar = gtk_header_bar_new();
    createToolBar(toolBar);
    gtk_window_set_titlebar(GTK_WINDOW(win), toolBar);
    
    GtkWidget* mainGrid;
    mainGrid = gtk_grid_new();
    createMainGrid(mainGrid);
    
    tab = gtk_notebook_new();
    
    GtkWidget *label1;
    label1 = gtk_label_new ("Canvas View");
    
    g_signal_connect (G_OBJECT(mainGrid), "clicked", G_CALLBACK (switch_page), (gpointer) tab);
    //g_signal_connect (G_OBJECT (tableGrid), "clicked", G_CALLBACK (switch_page), (gpointer) tab);
    gtk_notebook_append_page (GTK_NOTEBOOK (tab), mainGrid, label1);
    gtk_window_set_child (GTK_WINDOW (win), tab);
    gtk_widget_show(win);
  
}

int main (int argc, char *argv[])

{
  app = gtk_application_new ("com.example.GtkApplication",
                                             G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  

   // std::complex<double> v1{ 0.8, 0.6 };
    return g_application_run (G_APPLICATION (app), argc, argv);
}
