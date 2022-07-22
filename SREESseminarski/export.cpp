#include "export.h"
#include "systemModel.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <tuple>


const double eps{ 1e-10 };

void exportToLatex(SystemModel::SystemModel s) {
    std::ofstream myfile("main.tex");
    myfile << "\\documentclass[a4paper, 12pt]{article}" << std::endl;
    myfile << "\\usepackage[english]{babel}" << std::endl;
    myfile << "\\usepackage[letterpaper,top=2cm,bottom=2cm,left=3cm,right=3cm,marginparwidth=1.75cm]{geometry}" << std::endl;
    myfile << "\\usepackage{amsmath}" << std::endl;
    myfile << "\\usepackage{graphicx}" << std::endl;
    myfile << "\\usepackage[colorlinks=true, allcolors=blue]{hyperref}" << std::endl;
    myfile << "\\usepackage{multirow}" << std::endl;
    myfile << "\\usepackage{tabularx}" << std::endl;
    myfile << "\\usepackage{float}" << std::endl;
    myfile << "\\usepackage{makecell}" << std::endl;
    myfile << "\\title{PowerFlow Solver Export}" << std::endl;
    myfile << "\\author{}" << std::endl;
    myfile << "\\date{}" << std::endl;
    myfile << "\\begin{document}" << std::endl;
    myfile << "\\maketitle" << std::endl;
    myfile << "\\section*{Bus information}" << std::endl;
    myfile << "\\begin{table}[H]" << std::endl;
    myfile << "\t \\centering" << std::endl;
    myfile << "\t \\begin{tabular}{|c|c|c|c|}" << std::endl;
    myfile << "\t \t \\hline" << std::endl;
    myfile << "\t \t Bus \\textnumero& Bus type& Bus parameter& Values of parameter\\\\"<<std::endl;
    myfile << "\t \t \\hline" << std::endl;
    
    for (int i = 0; i < s.getNumberOfBuses(); i++)
    {
        myfile << "\t \t \\multirow{2}{*}{Bus " << i + 1 << "}&\\multirow{2}{*}";
        if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::PQ) {
            myfile << "{PQ}&Active power&" << s.getBus(i + 1).getActivePower().value() << "[p.u.]\\\\" << std::endl;
            myfile << "\t \t \\cline{ 3 - 4 }" << std::endl;
            myfile << "\t \t &&Reactive power&" << s.getBus(i + 1).getReactivePower().value() << "[p.u.]\\\\" << std::endl;  
        } 
        else if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::PV) {
            myfile << "{PV}&Active power&" << s.getBus(i + 1).getActivePower().value() << "[p.u.]\\\\" << std::endl;
            myfile << "\t \t \\cline{ 3 - 4 }" << std::endl;
            myfile << "\t \t && Voltage magnitude&" << s.getBus(i + 1).getVoltageMagnitude().value() << "[p.u.]\\\\" << std::endl;
        }
        else if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::Slack) {
            myfile << "{Slack}&Voltage magnitude&" << s.getBus(i + 1).getVoltageMagnitude().value() << "[p.u.]\\\\" << std::endl;
            myfile << "\t \t \\cline{ 3 - 4 }" << std::endl;
            myfile << "\t \t &&Voltage phase&" << s.getBus(i + 1).getVoltagePhase().value()*180/ 3.141592653589793238 << "[deg]\\\\" << std::endl;
        }
        myfile << "\t \t \\hline" << std::endl;
    }

    myfile << "\t \\end{tabular}" << std::endl;
    myfile << "\\end{table}" << std::endl;

    auto branches(s.getBranches());

    myfile << "\\section*{Line information}" << std::endl;
    myfile << "\\begin{table}[H]" << std::endl;
    myfile << "\t \\centering" << std::endl;
    myfile << "\t \\begin{tabular}{|c|c|c|c|c|}" << std::endl;
    myfile << "\t \t \\hline" << std::endl;
    myfile << "\t \t From bus \\textnumero&To bus \\textnumero&\\makecell{Series\\\\resistance}&\\makecell{Series\\\\reactance}&\\makecell{Shunt\\\\susceptance}\\\\" << std::endl;
    myfile << "\t \t \\hline" << std::endl;

    
    for (int i = 0; i < branches.size(); i++)
    {
        if (std::get<0>(branches[i]) == SystemModel::TypeOfBranch::Line) {
            myfile << "\t \t Bus " << int(std::get<1>(branches[i]))
                << "& Bus " << int(std::get<2>(branches[i]))
                << "& " << std::get<3>(branches[i]) << " [p.u]&"
                << std::get<4>(branches[i]) << " [p.u]&"
                << std::get<6>(branches[i]) << " [p.u]\\\\" << std::endl;
            myfile << "\t \t \\hline" << std::endl;
        }
    }
    myfile << "\t \\end{tabular}" << std::endl;
    myfile << "\\end{table}" << std::endl;

    
    myfile << "\\section*{Transformer information}" << std::endl;
    myfile << "\\begin{table}[H]" << std::endl;
    myfile << "\t \\centering" << std::endl;
    myfile << "\t \\begin{tabular}{|c|c|c|c|c|c|}" << std::endl;
    myfile << "\t \t \\hline" << std::endl;
    myfile << "\t \t From bus \\textnumero&To bus \\textnumero&\\makecell{Series\\\\resistance}&\\makecell{Series\\\\reactance}&\\makecell{Shunt\\\\conductance}&\\makecell{Shunt\\\\susceptance}\\\\" << std::endl;
    myfile << "\t \t \\hline" << std::endl;


    for (int i = 0; i < branches.size(); i++)
    {
        if (std::get<0>(branches[i]) == SystemModel::TypeOfBranch::Transformer) {
            myfile << "\t \t Bus " << int(std::get<1>(branches[i]))
                << "& Bus " << int(std::get<2>(branches[i]))
                << "& " << std::get<3>(branches[i]) << " [p.u]&"
                << std::get<4>(branches[i]) << " [p.u]&"
                << std::get<5>(branches[i]) << " [p.u]&"
                << std::get<6>(branches[i]) << " [p.u]\\\\" << std::endl;
            myfile << "\t \t \\hline" << std::endl;
        }
    }
    myfile << "\t \\end{tabular}" << std::endl;
    myfile << "\\end{table}" << std::endl;


    auto mat(s.getAdmittanceMatrix());
    std::vector<int> col, row;
    for (int i = 0; i < mat.size(); i++)
    {
        row.push_back(std::get<0>(mat.at(i)));
        col.push_back(std::get<1>(mat.at(i)));

    }
    
    int n(*std::max_element(row.begin(), row.end())), m(*std::max_element(col.begin(), col.end()));
    myfile << "\\section*{System admittance matrix}" << std::endl;
    myfile << "\\begin{center}" << std::endl;
    myfile << "$" << std::endl;
    myfile << "\\begin{bmatrix}" << std::endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            bool flag(0);
            for (int k = 0; k < mat.size(); k++)
            {
                if (std::get<0>(mat.at(k)) == i + 1 && std::get<1>(mat.at(k)) == j + 1)
                {
                    if (fabs(std::get<2>(mat.at(k)).real()) < eps) {
                        if (std::get<2>(mat.at(k)).imag() < eps) myfile << "-";
                        myfile << "j" << fabs(std::get<2>(mat.at(k)).imag()) << " ";
                    }
                    else {
                        myfile << std::get<2>(mat.at(k)).real();
                        if (fabs(std::get<2>(mat.at(k)).imag()) > eps) {
                            if (std::get<2>(mat.at(k)).imag() >= eps) myfile << "+";
                            else myfile << "-";
                            myfile << "j" << fabs(std::get<2>(mat.at(k)).imag()) << " ";
                        }
                        else myfile << " ";
                    }
                    flag = 1;
                }
            }
            if(!flag)
                myfile << 0;

            if (j != m - 1) myfile << " & ";
        }
        myfile << "\\\\" << std::endl;
    }
    myfile << "\\end{bmatrix}[p.u]" << std::endl;
    myfile << "$" << std::endl;
    myfile << "\\end{center}" << std::endl;

    myfile << std::endl << "\\end{document}" << std::endl;
}


void exportToHTML(SystemModel::SystemModel s) {
    std::ofstream myfile("main.html");
    myfile << "<!DOCTYPE html>" << std::endl;
    myfile << "<html>" << std::endl;
    myfile << "<head>" << std::endl;
    myfile << "<meta charset=\"utf-8\">" << std::endl;
    myfile << "<meta name=\"viewport\" content=\"width=device-width\">" << std::endl;
    myfile << "<title>PowerFlow Solver Export</title>" << std::endl;
    myfile << "<script type=\"text/javascript\" async" << std::endl;
    myfile << "src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.4/MathJax.js?config=TeX-MML-AM_CHTML\">" << std::endl;
    myfile << "</script>" << std::endl;
    myfile << "<link rel=\"stylesheet\" type=\"text/css\" " << std::endl;
    myfile << "href=\"https://cdn.rawgit.com/dreampulse/computer-modern-web-font/master/fonts.css\">" << std::endl;
    myfile << "<style>" << std::endl;
    myfile << "table, th, td{" << std::endl;
    myfile << "border: 1px solid black;" << std::endl;
    myfile << "margin-left: auto;" << std::endl;
    myfile << "margin-right: auto;" << std::endl;
    myfile << "}" << std::endl;
    myfile << "body {" << std::endl;
    myfile << "font-family: \"Lato\", sans-serif; " << std::endl << "}" << std::endl;
    myfile << "</style>" << std::endl;
    myfile << "</head> " << std::endl;
    myfile << "<body style=\"margin:0 10%;font-size:x-large\">" << std::endl;
    myfile << "<p style=\"text-align:center;font-size: xx-large;\">" << std::endl;
    myfile << "PowerFlow Solver Export" << std::endl;
    myfile << "</p>" << std::endl;
    myfile << "<p style=\"text-align:left\">" << std::endl;
    myfile << "Bus information: " << std::endl;
    myfile << "</p>" << std::endl;
    myfile << "<table style=\"text-align:center\">" << std::endl;
    myfile << "<tr>" << std::endl;
    myfile << "<th>Bus &numero;</th>" << std::endl;
    myfile << "<th>Bus type</th>" << std::endl;
    myfile << "<th>Bus parameter</th>" << std::endl;
    myfile << "<th>Values of parameter</th>" << std::endl;
    myfile << "</tr>" << std::endl;
    for (int i = 0; i < s.getNumberOfBuses(); i++)
    {
        myfile << "<tr>" << std::endl;
        myfile << "<td rowspan=\"2\">Bus " << i + 1 << "</td>";
        if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::PQ) {
            myfile << "<td rowspan=\"2\">PQ</td>" << std::endl;
            myfile << "<td>Active power</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getActivePower().value() << "[p.u.]</td>" << std::endl;
            myfile << "</tr>" << std::endl;
            myfile << "<tr>" << std::endl;
            myfile << "<td>Reactive power</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getReactivePower().value() << "[p.u.]</td>" << std::endl;
        }
        else if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::PV) {

            myfile << "<td rowspan=\"2\">PV</td>" << std::endl;
            myfile << "<td>Active power</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getActivePower().value() << "[p.u.]</td>" << std::endl;
            myfile << "</tr>" << std::endl;
            myfile << "<tr>" << std::endl;
            myfile << "<td>Voltage magnitude</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getVoltageMagnitude().value() << "[p.u.]</td>" << std::endl;
 
        }
        else if (s.getBus(i + 1).getTypeOfBus() == SystemModel::TypeOfBus::Slack) {

            myfile << "<td rowspan=\"2\">Slack</td>" << std::endl;
            myfile << "<td>Voltage magnitude</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getVoltageMagnitude().value() << "[p.u.]</td>" << std::endl;
            myfile << "</tr>" << std::endl;
            myfile << "<tr>" << std::endl;
            myfile << "<td>Voltage phase</td>" << std::endl;
            myfile << "<td>" << s.getBus(i + 1).getVoltagePhase().value() * 180 / 3.141592653589793238 << "[deg]</td>" << std::endl;
        }
        myfile << "</tr>" << std::endl;
    }
    myfile << "</table>" << std::endl;


    auto branches(s.getBranches());


    myfile << "Line information: " << std::endl;
    myfile << "</p>" << std::endl;
    myfile << "<table style=\"text-align:center\">" << std::endl;
    myfile << "<tr>" << std::endl;
    myfile << "<th>From bus &numero;</th>" << std::endl;
    myfile << "<th>To bus &numero;</th>" << std::endl;
    myfile << "<th>Series resistance</th>" << std::endl;
    myfile << "<th>Series reactance</th>" << std::endl;
    myfile << "<th>Shunt susceptance</th>" << std::endl;
    myfile << "</tr>" << std::endl;


    for (int i = 0; i < branches.size(); i++)
    {
        myfile << "<tr>" << std::endl;
        if (std::get<0>(branches[i]) == SystemModel::TypeOfBranch::Line) {
            myfile << "<td> Bus " << int(std::get<1>(branches[i])) << "</td>"
                << "<td> Bus " << int(std::get<2>(branches[i])) << "</td>"
                << "<td>" << std::get<3>(branches[i]) << " [p.u]</td>"
                << "<td>" << std::get<4>(branches[i]) << " [p.u]</td>"
                << "<td>" << std::get<6>(branches[i]) << " [p.u]</td>" << std::endl;
        }
        myfile << "</tr>" << std::endl;
    }
    myfile << "</table>" << std::endl;


    myfile << "Transformer information: " << std::endl;
    myfile << "</p>" << std::endl;
    myfile << "<table style=\"text-align:center\">" << std::endl;
    myfile << "<tr>" << std::endl;
    myfile << "<th>From bus &numero;</th>" << std::endl;
    myfile << "<th>To bus &numero;</th>" << std::endl;
    myfile << "<th>Series resistance</th>" << std::endl;
    myfile << "<th>Series reactance</th>" << std::endl;
    myfile << "<th>Shunt conductance</th>" << std::endl;
    myfile << "<th>Shunt susceptance</th>" << std::endl;
    myfile << "</tr>" << std::endl;


    for (int i = 0; i < branches.size(); i++)
    {
        myfile << "<tr>" << std::endl;
        if (std::get<0>(branches[i]) == SystemModel::TypeOfBranch::Transformer) {
            myfile << "<td> Bus " << int(std::get<1>(branches[i])) << "</td>"
                << "<td> Bus " << int(std::get<2>(branches[i])) << "</td>"
                << "<td>" << std::get<3>(branches[i]) << " [p.u]</td>"
                << "<td>" << std::get<4>(branches[i]) << " [p.u]</td>"
                << "<td>" << std::get<5>(branches[i]) << " [p.u]</td>"
                << "<td>" << std::get<6>(branches[i]) << " [p.u]</td>" << std::endl;
        }
        myfile << "</tr>" << std::endl;
    }
    myfile << "</table>" << std::endl;

   

    myfile << "<p style=\"text-align:left\">" << std::endl;
    myfile << "System admittance matrix: " << std::endl;
    myfile << "</p>" << std::endl;
    auto mat(s.getAdmittanceMatrix());
    std::vector<int> col, row;
    for (int i = 0; i < mat.size(); i++)
    {
        row.push_back(std::get<0>(mat.at(i)));
        col.push_back(std::get<1>(mat.at(i)));

    }

    int n(*std::max_element(row.begin(), row.end())), m(*std::max_element(col.begin(), col.end()));
    myfile << "\\begin{equation*}" << std::endl;
    myfile << "\\begin{bmatrix}" << std::endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            bool flag(0);
            for (int k = 0; k < mat.size(); k++)
            {
                if (std::get<0>(mat.at(k)) == i + 1 && std::get<1>(mat.at(k)) == j + 1)
                {
                    if (fabs(std::get<2>(mat.at(k)).real()) < eps) {
                        if (std::get<2>(mat.at(k)).imag() < eps) myfile << "-";
                        myfile << "j" << fabs(std::get<2>(mat.at(k)).imag()) << " ";
                    }
                    else {
                        myfile << std::get<2>(mat.at(k)).real();
                        if (fabs(std::get<2>(mat.at(k)).imag()) > eps) {
                            if (std::get<2>(mat.at(k)).imag() >= eps) myfile << "+";
                            else myfile << "-";
                            myfile << "j" << fabs(std::get<2>(mat.at(k)).imag()) << " ";
                        }
                        else myfile << " ";
                    }
                    flag = 1;
                }
            }
            if (!flag)
                myfile << 0;

            if (j != m - 1) myfile << " & ";
        }
        myfile << "\\\\" << std::endl;
    }
    myfile << "\\end{bmatrix}\\text{[p.u]}" << std::endl;
    myfile << "\\end{equation*}" << std::endl;
    myfile << std::endl << "</p>" << std::endl;
    myfile << std::endl << "</body>" << std::endl;
    myfile << std::endl << "</html>" << std::endl;
}