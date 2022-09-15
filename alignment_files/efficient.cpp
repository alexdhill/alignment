#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <sys/resource.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <algorithm>

long getTotalMemory();
void trim(std::string& str);
void generateInput(std::ifstream& in_file, std::string& s1, std::string& s2);
int getAlpha(char a, char b);
int findSplit(std::string& s1, std::string& s2);
struct alignment effAlign(std::string s1, std::string s2);
struct alignment quickAlign(std::string s1, std::string s2);

struct alignment
{
    std::string a1;
    std::string a2;
    unsigned int cost;
};

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

    // Generate Alignment
    struct alignment efficient_alignment = effAlign(s1, s2);
    std::ofstream out_file(argv[2]);
    out_file << efficient_alignment.cost << std::endl;  // NECESSARY OUTPUT
    out_file << efficient_alignment.a1 << std::endl << efficient_alignment.a2 << std::endl;  // NECESSARY OUTPUT

    // Stop CPU Timing and Calc
    gettimeofday(&end, 0);
    double totalmemory = getTotalMemory();
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double totaltime = seconds*1000 + microseconds*1e-3;

    // Output CPU Time and Mem Usage
    out_file << totaltime << std::endl;  // NECESSARY OUTPUT
    out_file << totalmemory << std::endl;  // NECESSARY OUTPUT
    return 0;
};

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
};

void trim(std::string& str)
{
    while (str.find_first_of(" \r\t\n")!=std::string::npos)
    {
        str.erase(str.find_first_of(" \r\t\n"), 1);
    }
};

// WARNING: NO INPUT VERIFICATION
void generateInput(std::ifstream& in_file, std::string& s1, std::string& s2)
{
    // Open Input
    std::string buffer;
    // Iterate Through Lines
    while (std::getline(in_file, buffer))
    {
        trim(buffer); // Helper for weird return characters
        if (
            !buffer.empty() &&
            buffer.find_first_not_of("0123456789")==std::string::npos)
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
};

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
};

int findSplit(std::string& s1, std::string& s2)
{
    std::string s1_right = s1.substr(s1.size()/2);
    std::string s1_left = s1.substr(0, s1.size()/2);

    // Create OPT_LEFT
    int** opt_left = new int*[s2.length()+1];
    for (int i = 0; i <= s2.length(); ++i)
    {
        opt_left[i] = new int[2];
    }
    // Fill Base Cases
    for (int i = 0; i <= s2.length(); ++i)
    {
        opt_left[i][0] = 30*i;
    }
    // Iterate to fill Left Opt
    for (int j = 1; j <= s1_left.length(); ++j)
    {
        opt_left[0][j%2] = 30*j;
        for (int i = 1; i <= s2.length(); ++i)
        {
            opt_left[i][j%2] = std::min(
                (opt_left[i-1][(j-1)%2])+getAlpha(s2[i-1], s1_left[j-1]),
                std::min(
                    opt_left[i-1][j%2],
                    opt_left[i][(j-1)%2])+30);
        }
    }

    // Create OPT_RIGHT
    int** opt_right = new int*[s2.length()+1];
    for (int i = 0; i <= s2.length(); ++i)
    {
        opt_right[i] = new int[2];
    }
    // Fill Base Cases
    for (int i = 0; i <= s2.length(); ++i)
    {
        opt_right[i][0] = 30*i;
    }

    // Iterate to Fill Right Opt
    for (int j = 1; j <= s1_right.length(); ++j)
    {
        opt_right[0][j%2] = 30*j;
        for (int i = 1; i <= s2.length(); ++i)
        {
            opt_right[i][j%2] = std::min(
                (opt_right[i-1][(j-1)%2])+getAlpha(s2[s2.length()-i], s1_right[s1_right.length()-j]),
                std::min(
                    opt_right[i-1][j%2],
                    opt_right[i][(j-1)%2])+30);
        }
    }

    // Sum Alignments
    unsigned int idx = 0;
    if (s1.length() == 1)
    {
        idx = opt_right[0][s1_right.length()%2];
        for (int i = 1; i <= s2.length(); ++i)
        {
            if (opt_right[i] < opt_right[idx]) idx = i;
        }
    } else 
    {
        int* opt_sum = new int[s2.length()+1];
        opt_sum[0] = opt_left[0][s1_left.length()%2] + opt_right[s2.length()][s1_right.length()%2];
        for (int i = 1; i <= s2.length(); ++i)
        {
            opt_sum[i] = opt_left[i][s1_left.length()%2] + opt_right[s2.length()-i][s1_right.length()%2];
            if (opt_sum[i] < opt_sum[idx]) idx = i;
        }
        delete[] opt_sum;
    }
    
    // Delete dynamic mem
    for (int i = 0; i <= s2.length(); ++i)
    {
        delete[] opt_right[i];
        delete[] opt_left[i];
    }
    delete[] opt_right;
    delete[] opt_left;

    // Return Minimum Alignment Cost
    return idx;
};

struct alignment effAlign(std::string s1, std::string s2)
{
    struct alignment efficient_align;
    if (s1.length() < 2)
    {
        efficient_align = quickAlign(s1, s2);
    } else if (s2.length() < 2)
    {
        struct alignment temp_align = quickAlign(s2, s1);
        efficient_align.a1 = temp_align.a2;
        efficient_align.a2 = temp_align.a1;
        efficient_align.cost = temp_align.cost;
    } else
    {
        int idx = findSplit(s1, s2);
        struct alignment align_right = effAlign(s1.substr(s1.length()/2), s2.substr(idx));
        struct alignment align_left = effAlign(s1.substr(0, s1.length()/2), s2.substr(0, idx));
        efficient_align.a1 = align_left.a1 + align_right.a1;
        efficient_align.a2 = align_left.a2 + align_right.a2;
        efficient_align.cost = align_left.cost + align_right.cost;
    }
    return efficient_align;
};

struct alignment quickAlign(std::string s1, std::string s2)
{
    struct alignment ralign;
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
            ralign.a2.insert(ralign.a2.begin(), s2[j-1]);
            ralign.a1.insert(ralign.a1.begin(), '_');
            --j;
        }
        else if (j == 0)
        {
            ralign.a1.insert(ralign.a1.begin(), s1[i-1]);
            ralign.a2.insert(ralign.a2.begin(), '_');
            --i;
        }
        else
        {
            if (opt[i][j]-getAlpha(s1[i-1], s2[j-1]) == opt[i-1][j-1])
            {
                ralign.a1.insert(ralign.a1.begin(), s1[i-1]);
                ralign.a2.insert(ralign.a2.begin(), s2[j-1]);
                --i; --j;
            }
            else if (opt[i][j]-30 == opt[i][j-1])
            {
                ralign.a2.insert(ralign.a2.begin(), s2[j-1]);
                ralign.a1.insert(ralign.a1.begin(), '_');
                --j;
            }
            else
            {
                ralign.a1.insert(ralign.a1.begin(), s1[i-1]);
                ralign.a2.insert(ralign.a2.begin(), '_');
                --i;
            }
        }
    }
    ralign.cost = opt[s1.length()][s2.length()];
    // Delete Opt Array
    for (int i = 0; i <= s1.length(); ++i)
    {
        delete[] opt[i];
    }
    delete[] opt;
    // Return Minimum Alignment
    return ralign;
}