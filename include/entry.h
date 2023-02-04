//
//  entry.h
//  NewtonRaphsonGui
//
//  Created by dzeni on 10. 8. 2022..
//

#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>


const char* getEntryText(GtkWidget* pEntry);
double getEntryDbl(GtkWidget* pEntry);
double getEntryInt(GtkWidget* pEntry);
void setEntryDbl(GtkWidget* pEntry, double num);
void setEntryInt(GtkWidget* pEntry, int num);
void setEntryText(GtkWidget* pEntry, const char* text, int len);
