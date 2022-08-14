//
//  initList.cpp
//  NewtonRaphsonGui
//
//  Created by dzeni on 7. 8. 2022..
//

#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <tuple>
#include <iomanip>
#include <cmath>
#include "initList.h"

//void dblFormater (GtkTreeViewColumn* col, GtkCellRenderer* renderer, GtkTreeModel* model, GtkTreeIter* iter, gpointer user_data)
//{
//    double val;
//    gpointer iColumn = user_data;
//    gtk_tree_model_get(model, iter, iColumn, &val, -1);
//    val = std::ceil(val * 1000.0) / 1000.0;
//    g_object_set(renderer, "text", val, NULL);
//}

void initSlackList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> slackValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Bus ID", renderer, "text", BUS_ID, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Voltage Magnitude", renderer, "text", VOLTAGE_MAGNITUDE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
//    gpointer par1 = (gpointer) VOLTAGE_MAGNITUDE;
//    gtk_tree_view_column_set_cell_data_func(column, renderer, dblFormater, par1, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Voltage Phase", renderer, "text", VOLTAGE_PHASE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
//    par1 = (gpointer) VOLTAGE_PHASE;
//    gtk_tree_view_column_set_cell_data_func(column, renderer, dblFormater, par1, NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < slackValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set (store, &iter, BUS_ID, std::get<0>(slackValues[i]), VOLTAGE_MAGNITUDE, std::get<1>(slackValues[i]), VOLTAGE_PHASE, std::get<2>(slackValues[i]), COLOR, std::get<3>(slackValues[i]), -1);
    }
    g_object_unref(store);
}

void initPvList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> pvValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Bus ID", renderer, "text", BUS_ID, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_alignment(renderer, 1.0f, 1.0f);
    column = gtk_tree_view_column_new_with_attributes("Voltage Magnitude", renderer, "text", VOLTAGE_MAGNITUDE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_alignment(column, 0.5f);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Active Power", renderer, "text", ACTIVE_POWER, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    renderer = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < pvValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set (store, &iter, BUS_ID, std::get<0>(pvValues[i]), VOLTAGE_MAGNITUDE, std::get<1>(pvValues[i]), ACTIVE_POWER, std::get<2>(pvValues[i]),COLOR, std::get<3>(pvValues[i]), -1);
    }
    g_object_unref(store);
}

void initPqList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> pqValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Bus ID", renderer, "text", BUS_ID, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Active Power", renderer, "text", ACTIVE_POWER, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Reactive Power", renderer, "text", REACTIVE_POWER, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < pqValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set (store, &iter, BUS_ID, std::get<0>(pqValues[i]), ACTIVE_POWER, std::get<1>(pqValues[i]), REACTIVE_POWER, std::get<2>(pqValues[i]), COLOR, std::get<3>(pqValues[i]), -1);
    }
    g_object_unref(store);
}

void initLineList(GtkWidget *list, std::vector<std::tuple<int, int, double, double, double, const char*, double, double, double, double>> lineValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Start Bus", renderer, "text", START_BUS, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("End Bus", renderer, "text", END_BUS, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Re(Z)", renderer, "text", REAL_IMPEDANCE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Im(Z)", renderer, "text", IMAG_IMPEDANCE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Im(Y_sh)", renderer, "text", SHUNT, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < lineValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, START_BUS, std::get<0>(lineValues[i]), END_BUS, std::get<1>(lineValues[i]), REAL_IMPEDANCE, std::get<2>(lineValues[i]),IMAG_IMPEDANCE,std::get<3>(lineValues[i]), SHUNT, std::get<4>(lineValues[i]), COLOR,std::get<5>(lineValues[i]) , -1);
    }
    g_object_unref(store);
}

void initBatteryList(GtkWidget *list, std::vector<std::tuple<int, double, const char*, const char*, double, double>> batteryValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Connected Bus", renderer, "text", BUS_ID, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Capacity", renderer, "text", CAPACITY, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Configuration", renderer, "text", CONFIGURATION, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < batteryValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, BUS_ID, std::get<0>(batteryValues[i]), CAPACITY, std::get<1>(batteryValues[i]), CONFIGURATION, std::get<2>(batteryValues[i]),COLOR,std::get<3>(batteryValues[i]), -1);
    }
    g_object_unref(store);
}

void initTransformatorList(GtkWidget *list, std::vector<std::tuple<int, int, double, double, double, double, const char*, double, double>> transValues)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Start Bus", renderer, "text", START_BUS, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    //gtk_tree_view_column_set_min_width(column, 5);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 60);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("End Bus", renderer, "text", END_BUS, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Re(Z)", renderer, "text", REAL_IMPEDANCE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Im(Z)", renderer, "text", IMAG_IMPEDANCE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Re(Y_sh)", renderer, "text", SHUNT_REAL, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Im(Y_sh)", renderer, "text", SHUNT, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("Color", renderer, "text", COLOR, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_reorderable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    GtkTreeIter iter;
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE,G_TYPE_DOUBLE,G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

    for(int i = 0; i < transValues.size(); i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, START_BUS, std::get<0>(transValues[i]), END_BUS, std::get<1>(transValues[i]), REAL_IMPEDANCE, std::get<2>(transValues[i]),IMAG_IMPEDANCE,std::get<3>(transValues[i]), SHUNT_REAL, std::get<4>(transValues[i]), SHUNT, std::get<5>(transValues[i]),COLOR,std::get<6>(transValues[i]) , -1);
    }
    g_object_unref(store);
}
