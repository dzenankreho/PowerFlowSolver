//
//  initList.h
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

#ifndef initList_h
#define initList_h

enum
{
    BUS_ID,
    VOLTAGE_MAGNITUDE,
    VOLTAGE_PHASE,
    ACTIVE_POWER,
    REACTIVE_POWER,
    START_BUS,
    END_BUS,
    REAL_IMPEDANCE,
    IMAG_IMPEDANCE,
    SHUNT,
    SHUNT_REAL,
    CAPACITY,
    COLOR,
    CONFIGURATION,
    N_COLUMNS
};

void dblFormater(GtkTreeViewColumn* col, GtkCellRenderer* renderer, GtkTreeModel* model, GtkTreeIter* iter, gpointer user_data);
void initSlackList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> slackValues);
void initPvList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> pvValues);
void initPqList(GtkWidget *list, std::vector<std::tuple<int, double, double, const char*, double, double>> pqValues);
void initLineList(GtkWidget *list, std::vector<std::tuple<int, int, double, double, double, const char*, double, double, double, double>> lineValues);
void initBatteryList(GtkWidget *list, std::vector<std::tuple<int, double, const char*, const char*, double, double>> batteryValues);
void initTransformatorList(GtkWidget *list, std::vector<std::tuple<int, int, double, double, double, double, const char*, double, double>> transValues);

#endif /* initList_h */
