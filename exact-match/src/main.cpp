#include <iostream>
#include <vector>
#include <string>
#include "../include/exactMatch.cpp"
#include <chrono>
#include <sstream>
#include <fstream>

using namespace std;

int main() {
    string s; // = "xyabcxabcxadcdqfeg";
    string p; // = "abcxabcxa";
    //string alphabet = "abcdefghijklmnopqrstuvwxyz";
    string alphabet = "acgt";

    // user input
    //cout << "Type in the string you want to search the pattern for:" << endl;
    //cin >> s;
    //cout << "Type in the pattern string:" << endl;
    //cin >> p;

    // test search for words in shapkespear complete work.
    //ifstream openFileStream("../test/t8.shakespeare.txt");
    ifstream openFileStream("../test/chr20.fa");
    stringstream buffer;
    buffer << openFileStream.rdbuf();
    s = buffer.str();

    //ifstream wordsFile("../test/google-10000-english-usa-no-swears.txt");
    ifstream wordsFile("../test/5000-sequences.reformatted.txt");
    ofstream outFile("../test/runtime-for-random-sequences-v2.txt");
    while (wordsFile >> p)
    {
        BoyerMoore bm(p, alphabet);
        auto timeStart = chrono::high_resolution_clock::now(); 
        vector<int> m = bm.match(s);
        auto timeEnd = chrono::high_resolution_clock::now(); 
        auto duration = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 

        // KMP search
        KMP kmp(p);
        timeStart = chrono::high_resolution_clock::now(); 
        vector<int> kmpResult = kmp.match(s);
        timeEnd = chrono::high_resolution_clock::now(); 
        auto duration2 = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 

        timeStart = chrono::high_resolution_clock::now(); 
        vector<int> kmpResult2 = kmp.matchUsingNextArray(s);
        timeEnd = chrono::high_resolution_clock::now(); 
        auto duration3 = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 

        outFile << p.size() << '\t' << duration.count() << '\t' 
                << duration2.count() << '\t' << duration3.count() << '\n';
    }
    wordsFile.close();
    outFile.close();

//    BoyerMoore bm(p, alphabet);
//    auto timeStart = chrono::high_resolution_clock::now(); 
//    vector<int> m = bm.match(s);
//    auto timeEnd = chrono::high_resolution_clock::now(); 
//    auto duration = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 
//    // KMP search
//    KMP kmp(p);
//    timeStart = chrono::high_resolution_clock::now(); 
//    vector<int> kmpResult = kmp.match(s);
//    timeEnd = chrono::high_resolution_clock::now(); 
//    auto duration2 = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 
//
//    timeStart = chrono::high_resolution_clock::now(); 
//    vector<int> kmpResult2 = kmp.matchUsingNextArray(s);
//    timeEnd = chrono::high_resolution_clock::now(); 
//    auto duration3 = chrono::duration_cast<chrono::milliseconds>(timeEnd - timeStart); 
//
//    cout << "p: " << p << endl;
//    for (const auto &value : m)
//    {
//        cout << value << " ";
//    }
//    cout << endl << "Occurrence: " << m.size() << endl;
//    cout << "BoyerMoore Run Time: "
//        << duration.count() << " ms" << endl;
//    for (const auto &value : kmpResult)
//    {
//        cout << value << " ";
//    }
//    cout << endl << "Occurrence: " << kmpResult.size() << endl;
//    cout << "KMP Run Time: "
//        << duration2.count() << " ms" << endl; 
//    for (const auto &value : kmpResult2)
//    {
//        cout << value << " ";
//    }
//    cout << endl << "Occurrence: " << kmpResult2.size() << endl;
//    cout << "KMP vanilla Run Time: "
//        << duration3.count() << " ms" << endl; 
    return 0;
}