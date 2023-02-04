//
//  entry.cpp
//  NewtonRaphsonGui
//
//  Created by dzeni on 10. 8. 2022..
//

#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "entry.h"

const char* getEntryText(GtkWidget* pEntry)
{
    GtkEntryBuffer* buffer = gtk_entry_get_buffer (GTK_ENTRY(pEntry));
    const char* text = gtk_entry_buffer_get_text (buffer);
    return text;
}

double getEntryDbl(GtkWidget* pEntry)
{
    const char *val1 = getEntryText(pEntry);
    double br = atof(val1);
    return br;
}
double getEntryInt(GtkWidget* pEntry)
{
    const char *val1 = getEntryText(pEntry);
    int br = atoi(val1);
    return br;
}

void setEntryDbl(GtkWidget* pEntry, double num)
{
    GtkEntryBuffer* buffer = gtk_entry_get_buffer (GTK_ENTRY(pEntry));
    char output[50];

    snprintf(output, 50, "%.4f", num);
    gtk_entry_buffer_set_text (buffer, output, 50);
}

void setEntryInt(GtkWidget* pEntry, int num)
{
    GtkEntryBuffer* buffer = gtk_entry_get_buffer (GTK_ENTRY(pEntry));
    char output[50];

    snprintf(output, 50, "%d", num);
    gtk_entry_buffer_set_text (buffer, output, 50);
}

void setEntryText(GtkWidget* pEntry, const char* text, int len)
{
    GtkEntryBuffer* buffer = gtk_entry_get_buffer (GTK_ENTRY(pEntry));
    
    size_t nch = len;
    if(nch <= 0)
        nch = strlen(text);
    
    gtk_entry_buffer_set_text (buffer, text, (int)nch);
}
