#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <chrono>
//#include <regex>
#include <sstream>
#include <fstream>

using namespace std;

// Z array, maximum length of prefix of s[i..n] which is also a prefix of s
vector<int> ZArray(const string& s)
{
    size_t s_len = s.size();
    if(s_len == 0) 
    {
        cerr << "Input string is empty!" << endl;
        throw "Input string is empty!";
    }

    vector<int> z(s_len);
    unsigned int l = 0, r = 0;
    z[0] = 0; //s_len; set to s_len does not affect BoyerMoore, but affect KMP results.
    for (size_t i = 1; i < s_len; i++)
    {
        // Case 1
        if ( i > r)
        {
            l = r = i;
            while (r < s_len && s[r] == s[r-l])
                r++;
            z[i] = r - l;
            r--;
        }
        else
        // Case 2
        {
            // Case 2a, use previous calculated Z values
            if (i + z[i-l] <= r)
            {
                z[i] = z[i-l];
            }
            else
            // Case 2b, need to compare chars after right boundary
            {
                l = i;
                while (r < s_len && s[r] == s[r-l])
                    r++;
                z[i] = r - l;
                r--;
            }
        }
    }
    return z;
}

// Pattern match using Z algorithm. T: O(n+m), S: O(n+m)
vector<int> ZMatch(const string& pattern, const string& str)
{
    vector<int> matchIndex;
    int p_len = pattern.size();
    if (p_len == 0)
    {
        cerr << "Input pattern is empty!" << endl;
        throw "Input pattern is empty";
    }
    string combined = pattern + "$" + str;
    vector<int> z = ZArray(combined);
    for (size_t i = 0; i < z.size(); i++)
        if (z[i] == p_len)
            matchIndex.push_back(i-p_len-1);
    return matchIndex;
}

void reverseString(string& s)
{
    size_t str_len = s.size();
    for (size_t i = 0; i < str_len / 2; i++)
        swap(s[i], s[str_len-i-1]);
}

// N array, maximum length of suffix of s[0..i] which is also a suffix of s
vector<int> NArray(const string& s)
{
    string s_copy(s);
    reverse(s_copy.begin(), s_copy.end());
    vector<int> z = ZArray(s_copy);
    reverse(z.begin(), z.end());
    return z;
}

// L array, largest index j such that p[i..n] is a suffix of p[1..j]
// (strong suffix rule) L' array, largest index j such that p[i..n] is a suffix of p[1..j] and p[i-1] != p[j-1]
void LPrimeArray(vector<int> n, vector<int>& Lp, vector<int>& L)
{
    int nLen = n.size();
    int i;
    for (int j = 0; j < nLen - 1; j++)
    {
        i = nLen - n[j];
        // if i == nLen - 1, does it degenerate to bad character rule?
        if (i < nLen)
            Lp[i] = j;
    }
    // L(i) = max(L'(i), L(i-1)), because if p[j-1] = p[i-1] then L(i) = L(i-1), else then L(i) = L'(i) 
    L[0] = Lp[0];
    for (int i = 1; i < nLen; i++)
    {
        L[i] = (L[i-1] > Lp[i] ? L[i-1] : Lp[i]);
    }
}

// l' array, maximum length of suffix of p[i..n] which is also a prefix of p
// l' equal to largest j <= |p[i..n]| = n-i+1 such that Nj = j
vector<int> SmallLPrimeArray(vector<int> n)
{
    int nLen = n.size();
    vector<int> lp(n.size(), 0);
    int i = 0;
    for (int j = nLen-1; j >= 0; j--)
    {
        if (n[j] == j + 1)
        {
            while (i <= nLen - (j + 1))
                lp[i++] = j + 1;
        }
    }
    return lp;
}

// Good suffix mismatch using the L' array and l' array
// if offset i == nLen - 1, return 0; no matched suffix
// otherwise, if L'[i+1] >= 0; shift by nLen - L'[i+1] - 1
// otherwise, shift by nLen - l'[i+1] - 1
int goodSuffixMismatch(int i, vector<int>& Lprime, vector<int>& lprime)
{
    int nLen = Lprime.size();
    if(i >= nLen) 
    {
        cerr << "offset out of pattern end!" << endl;
        throw "offset out of pattern end!";
    }
    if (i == nLen - 1)
        return 0;
    i++; // i is a mismatch, shift to right by 1 to the matched char
    if (Lprime[i] >= 0)
        return nLen - Lprime[i] - 1;
    return nLen - lprime[i] - 1;
}

// dense bad character table given pattern and list of characters
unordered_map<char, vector<int>> badCharTable(string& p, string& charStr)
{
    unordered_map<char, vector<int>> tab;
    vector<int> shifts;
    int shift;

    for (const auto& c : charStr)
    {
        shifts.clear();
        shift = 0;
        for (const auto& pChar : p)
        {
            if (c == pChar)
                shift = 0;
            else
                shift++;
            shifts.push_back(shift);
        }
        tab[c] = shifts;
    }
    return tab;
}

// Boyer-Moore pattern match, match the string from right to left
class BoyerMoore
{
    public:

    unordered_map<char, vector<int>> badCharTab; // precalcualted shift table for bad character rule
    vector<int> nArray;
    vector<int> bigLArray;
    vector<int> bigLPrimeArray;
    vector<int> smallLPrimeArray;
    vector<int> goodSuffixTab; // precalculated shift table for good suffix rule, idea is put as much as calculation in the preprocessing step
    string pattern;
    int skipWhenMatched;
    BoyerMoore(string p, string alphabet)
    {
        pattern = p;
        nArray = NArray(p);
        bigLArray.assign(nArray.size(), -1);
        bigLPrimeArray.assign(nArray.size(), -1);
        LPrimeArray(nArray, bigLPrimeArray, bigLArray);
        smallLPrimeArray = SmallLPrimeArray(nArray);
        badCharTab = badCharTable(p, alphabet);
        skipWhenMatched = smallLPrimeArray.size() - smallLPrimeArray[1];
        skipWhenMatched = max(1, skipWhenMatched);

        goodSuffixTab.assign(p.size(), 0);
        for (size_t i = 0; i < p.size(); i++)
        {
            goodSuffixTab[i] = goodSuffixMismatch(i, bigLPrimeArray, smallLPrimeArray);
        }
    }
    
    int badCharSkip(int i, char c)
    {
        // if the char does not exists in the pattern!
        if (badCharTab.find(c) == badCharTab.end())
            return i + 1;
        return badCharTab[c][i];
    }

    //int goodSuffixSkip(int i)
    //{
    //    return goodSuffixTab[i];
    //}

    vector<int> match(const string& targetStr)
    {
        vector<int> occurrence;
        if (targetStr.size() < pattern.size())
        {
            throw "Pattern is longer than the target string!";
        }
        int j = pattern.size() - 1;
        int strLen = targetStr.size();
        bool matched;
        int shift;
        while (j < strLen)
        {
            matched = true;
            for (int i = pattern.size() - 1; i >= 0; i--)
            {
                if (pattern[i] != targetStr[j - pattern.size() + 1 + i])
                {
                    shift = max(badCharSkip(i, targetStr[j - pattern.size() + 1 + i]), goodSuffixTab[i]);
                    matched = false;
                    break;
                }
            }
            if (matched)
            {
                occurrence.push_back(j - pattern.size() + 1);
                shift = skipWhenMatched;
            }
            j += shift;
        }
        return occurrence;
    }
};

// KMP pattern match, match the string from left to right
class KMP
{
    public:
    vector<int> spPrimeArray;
    vector<int> mismatchSkipTab; // Z based mismatch skip table
    vector<int> lps;  // longest prefix suffix array, KMP vanilla preprocessing
    string pattern;
    int skipWhenMatched; // constant skip when a match occurs
    KMP(string p)
    {
        vector<int> z = ZArray(p);
        spPrimeArray = SpPrimeArray(z);
        skipWhenMatched = spPrimeArray.size() - spPrimeArray[spPrimeArray.size() - 1];
        pattern = p;
        
        mismatchSkipTab.assign(spPrimeArray.size(), 0);
        for (size_t i = 0; i < spPrimeArray.size(); i++)
            mismatchSkipTab[i] = mismatchSkip(i);
        
        // initialize LPS array
        lps = nextArray(p);
    }

    // sp' array, the length of the longest suffix of s[1..i] which is also a prefix of s with extra condition that s[i+1] != s[sp'(i)+1]. sp' can be calculated from Z array. 
    // also called "longest prefix suffix"
    // This is the stronger KMP rule. see Theorem 2.3.2.
    vector<int> SpPrimeArray(vector<int>& z)
    {
        vector<int> spp(z.size(), 0);
        int j = z.size();
        int i;
        while (j-- > 0)
        {
            i = j + z[j] - 1; // this is tricky, but if you think of Z array as the "longest prefix prefix".
            spp[i] = z[j];
        }
        return spp;
    }

    // if a mismatch happens at i, shift the pattern by i - sp'[i-1]
    int mismatchSkip(int i)
    {
        if (i == 0)
        {   return 1; }
        else        
        {
            return i - spPrimeArray[i-1];
        }
        
    }

    vector<int> match(const string& targetStr)
    {
        vector<int> occurrence;
        if (targetStr.size() < pattern.size())
        {
            throw "Pattern is longer than the target string!";
        }
        int j = 0;
        int rightBound = targetStr.size() - pattern.size();
        int shift_j, shift_i;
        bool matched;
        shift_i = 0; // matched suffix of p[1..i] need not to be compared again
        while (j <= rightBound)
        {
            matched = true;
            shift_j = 1;
            for (size_t i = shift_i; i < pattern.size(); i++)
            {
                if (pattern[i] != targetStr[j + i])
                {
                    shift_j = mismatchSkipTab[i];
                    if (i > 0)
                        shift_i = spPrimeArray[i-1];
                    matched = false;
                    break;
                }
            }
            if (matched)
            {
                occurrence.push_back(j);
                shift_j = skipWhenMatched;
                shift_i = spPrimeArray[spPrimeArray.size()-1];
            }
            j += shift_j;
        }
        return occurrence;
    }

    // there is another way to implement the KMP algorithm and calculate the lps array, some textbook also refer this to the next array.
    vector<int> nextArray(const string& p)
    {
        vector<int> lps(p.size(), 0); // lps[0] = 0
        int len = 0; // the longest length of prefix suffix for p[0..i]
        int i = 1; // loop from 1 to n-1 to get the lps array 
        int pLen = p.size();
        while (i < pLen)
        {
            if (p[i] == p[len])
            {
                len++;
                lps[i++] = len;  // increment len and i when there is a match between them
            }
            else
            {
                if (len != 0)     // decrease len when there is a mismatch
                    len = lps[len - 1]; // this is the most tricky part, similar to the trick used in the search step, do not increase i here.
                else
                    lps[i++] = 0;
            }
        }
        return lps;
    }

    vector<int> matchUsingNextArray(const string& targetStr)
    {
        vector<int> occurrence;
        int i, j;
        i = j = 0;
        int tLen = targetStr.size();
        int pLen = pattern.size();
        while (i < tLen)
        {
            if (pattern[j] == targetStr[i])
            {
                if (j == pLen - 1)  // this indicates the pattern is found in the target string
                {
                    occurrence.push_back(i - pLen + 1);
                    j = lps[j];
                }
                else
                {   j++; }
                i++;
            }
            else
            {
                if (j > 0)
                    j = lps[j - 1]; //
                else
                    i++;
            }
        }
        return occurrence;
    }
};