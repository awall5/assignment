#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <algorithm>
#include <unordered_map>
using namespace std;

#include "json.hpp"
using json = nlohmann::json;

using int128 = __int128_t;

int128 strToInt128(const string &s, int base)
{
    int128 res = 0;
    for (char c : s)
    {
        int val;
        if (isdigit(c))
            val = c - '0';
        else if (isalpha(c))
            val = tolower(c) - 'a' + 10;
        else
            throw invalid_argument("Invalid digit");
        if (val >= base)
            throw invalid_argument("Digit exceeds base");
        res = res * base + val;
    }
    return res;
}

void printInt128(int128 num)
{
    if (num == 0)
    {
        cout << "0";
        return;
    }
    string out;
    bool neg = false;
    if (num < 0)
    {
        neg = true;
        num = -num;
    }
    while (num > 0)
    {
        out += '0' + (num % 10);
        num /= 10;
    }
    if (neg)
        out += '-';
    reverse(out.begin(), out.end());
    cout << out;
}

int128 modInverse(int128 a, int128 m)
{
    int128 m0 = m, t, q;
    int128 x0 = 0, x1 = 1;
    if (m == 1)
        return 0;
    while (a > 1)
    {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    return (x1 + m0) % m0;
}

int128 lagrangeInterpolation(vector<pair<int128, int128>> &points, int128 mod)
{
    int128 secret = 0;
    int k = points.size();
    for (int i = 0; i < k; ++i)
    {
        int128 xi = points[i].first;
        int128 yi = points[i].second;
        int128 num = 1, den = 1;
        for (int j = 0; j < k; ++j)
        {
            if (i == j)
                continue;
            int128 xj = points[j].first;
            num = (num * (-xj + mod)) % mod;
            den = (den * (xi - xj + mod)) % mod;
        }
        int128 inv = modInverse(den, mod);
        secret = (secret + (yi * num % mod) * inv % mod) % mod;
    }
    return (secret + mod) % mod;
}

void combinations(vector<pair<int128, int128>> &arr, int k, int start, vector<pair<int128, int128>> &curr,
                  vector<vector<pair<int128, int128>>> &result)
{
    if (curr.size() == k)
    {
        result.push_back(curr);
        return;
    }
    for (int i = start; i < arr.size(); ++i)
    {
        curr.push_back(arr[i]);
        combinations(arr, k, i + 1, curr, result);
        curr.pop_back();
    }
}

int main()
{
    ifstream file("input.json");
    if (!file.is_open())
    {
        cerr << "Could not open input.json" << endl;
        return 1;
    }
    json j;
    file >> j;

    int N = j["keys"]["n"];
    int K = j["keys"]["k"];

    vector<pair<int128, int128>> points;
    for (auto &el : j.items())
    {
        if (el.key() == "keys")
            continue;
        int x = stoi(el.key());
        int base = stoi(el.value()["base"].get<string>());
        string valStr = el.value()["value"].get<string>();
        try
        {
            int128 y = strToInt128(valStr, base);
            points.push_back({x, y});
        }
        catch (...)
        {
            cerr << "Skipping invalid share: " << valStr << " in base " << base << endl;
        }
    }

    const int128 MOD = 9223372036854775783LL;

    vector<vector<pair<int128, int128>>> combs;
    vector<pair<int128, int128>> current;
    combinations(points, K, 0, current, combs);

    unordered_map<string, int> freq;
    map<string, int128> strToVal;

    for (auto &group : combs)
    {
        try
        {
            int128 res = lagrangeInterpolation(group, MOD);
            stringstream ss;
            ss << (long long)res;
            freq[ss.str()]++;
            strToVal[ss.str()] = res;
        }
        catch (...)
        {
            continue;
        }
    }

    string mostFreq;
    int maxCount = 0;
    for (auto &[s, count] : freq)
    {
        if (count > maxCount)
        {
            maxCount = count;
            mostFreq = s;
        }
    }

    cout << "The secret (constant term) is: ";
    printInt128(strToVal[mostFreq]);
    cout << endl;
    return 0;
}
