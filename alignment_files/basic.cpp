#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <sys/resource.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

long getTotalMemory();
void trim(std::string& str);
void generateInput(std::ifstream& in_file, std::string& s1, std::string& s2);
int getAlpha(char a, char b);
int align(std::string& s1, std::string& s2, std::string& a1, std::string& a2);

int main(int argc, char* argv[])
{
    // Start CPU Timer
    struct timeval begin, end;
    gettimeofday(&begin, 0);
    // Verify Arguments
    if (argc != 3)
    {
        std::cout << "USAGE: ./basic.sh <input.txt> <output.txt>" << std::endl;
        return 1;
    }
    // Generate Input Strings
    std::ifstream in_file(argv[1]);
    std::string s1, s2;
    generateInput(in_file, s1, s2);
    in_file.close();
    // std::cout << s1 << std::endl << s2 << std::endl; // Debugging Only
    // std::cout << "len s1: " << s1.length() << std::endl;
    // std::cout << "len s2: " << s2.length() << std::endl;
    // Generate Alignment
    std::string a1, a2;
    int cost = align(s1, s2, a1, a2);
    std::ofstream out_file(argv[2]);
    out_file << cost << std::endl;
    out_file << a1 << std::endl << a2 << std::endl;
    // Stop CPU Timing and Calc
    gettimeofday(&end, 0);
    double totalmemory = getTotalMemory();
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double totaltime = seconds*1000 + microseconds*1e-3;
    // Output CPU Time
    out_file << totaltime << std::endl;
    // Output Mem Usage
    out_file << totalmemory << std::endl;
    return 0;
}

long getTotalMemory()
{
    struct rusage usage;
    int returnCode = getrusage(RUSAGE_SELF, &usage);
    if (returnCode==0)
    {
        return usage.ru_maxrss;
    }
    else
    {
        return -1;
    }
}

void trim(std::string& str)
{
    while (str.find_first_of(" \r\t\n")!=std::string::npos)
    {
        str.erase(str.find_first_of(" \r\t\n"), 1);
    }
}

// WARNING: NO INPUT VERIFICATION
void generateInput(std::ifstream& in_file, std::string& s1, std::string& s2)
{
    // Open Input
    std::string buffer;
    // Iterate Through Lines
    while (std::getline(in_file, buffer))
    {
        trim(buffer); // Helper for weird return characters
        if (!buffer.empty() && buffer.find_first_not_of("0123456789")==std::string::npos) // is digit
        {
            size_t idx = atoi(buffer.c_str());
            if (s2.empty())
            {
                s1.insert(idx+1, s1);
            }
            else
            {
                s2.insert(idx+1, s2);
            }
        }
        // Handle New Strings
        else{
            s1.empty()?s1=buffer:s2=buffer;
        }
    }
    return;
}

int getAlpha(char a, char b)
{
    // Returns Match/Mismatch Costs
    if (a == b)
    {
        return 0;
    }
    else if ((a=='A' && b=='C') || (a=='C' && b=='A'))
    {
        return 110;
    }
    else if ((a=='A' && b=='G') || (a=='G' && b=='A'))
    {
        return 48;
    }
    else if ((a=='A' && b=='T') || (a=='T' && b=='A'))
    {
        return 94;
    }
    else if ((a=='G' && b=='C') || (a=='C' && b=='G'))
    {
        return 118;
    }
    else if ((a=='G' && b=='T') || (a=='T' && b=='G'))
    {
        return 110;
    }
    else if ((a=='T' && b=='C') || (a=='C' && b=='T'))
    {
        return 48;
    }
    else
    {
        // Bad Characters
        return -1;
    }
}

int align(std::string& s1, std::string& s2, std::string& a1, std::string& a2)
{
    // Create OPT Array
    int** opt = new int*[s1.length()+1];
    for (int i = 0; i <= s1.length(); ++i)
    {
        opt[i] = new int[s2.length()+1];
    }
    // Fill Base Cases
    for (int i = 0; i <= s1.length(); ++i)
    {
        opt[i][0] = 30*i;
    }
    for (int j = 0; j <= s2.length(); ++j)
    {
        opt[0][j] = 30*j;
    }
    // Iterate to Fill OPT
    for (int i = 1; i <= s1.length(); ++i)
    {
        for (int j = 1; j <= s2.length(); ++j)
        {
            opt[i][j] = std::min((opt[i-1][j-1]+getAlpha(s1[i-1], s2[j-1])), (std::min(opt[i-1][j], opt[i][j-1])+30));
        }
    }
    // Backtrace Alignment
    int i = s1.length(); int j = s2.length();
    while (i > 0 || j > 0)
    {
        if (i == 0)
        {
            a2.insert(a2.begin(), s2[j-1]);
            a1.insert(a1.begin(), '_');
            --j;
        }
        else if (j == 0)
        {
            a1.insert(a1.begin(), s1[i-1]);
            a2.insert(a2.begin(), '_');
            --i;
        }
        else
        {
            if (opt[i][j]-getAlpha(s1[i-1], s2[j-1]) == opt[i-1][j-1])
            {
                a1.insert(a1.begin(), s1[i-1]);
                a2.insert(a2.begin(), s2[j-1]);
                --i; --j;
            }
            else if (opt[i][j]-30 == opt[i][j-1])
            {
                a2.insert(a2.begin(), s2[j-1]);
                a1.insert(a1.begin(), '_');
                --j;
            }
            else
            {
                a1.insert(a1.begin(), s1[i-1]);
                a2.insert(a2.begin(), '_');
                --i;
            }
        }
    }
    int cost = opt[s1.length()][s2.length()];
    // Delete Opt Array
    for (int i = 0; i <= s1.length(); ++i)
    {
        delete[] opt[i];
    }
    delete[] opt;
    // Return Minimum Alignment Cost
    return cost;
}