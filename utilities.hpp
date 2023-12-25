// utilities.hpp
//
// Purpose: 1. Defines several helper functions used in other classes.
// 
// @author Yuanting Li
// @version 1.0 2023/12/21

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <fstream>
#include <random>

#include "products.hpp"

using namespace std;

// Color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

// get the system time and return in milliseconds format (e.g. 2023-12-23 22:42:44.260)
std::string getTime()
{
    using namespace std::chrono;

    // Get current time as time_point
    system_clock::time_point now = system_clock::now();

    // Convert to time_t for conversion to tm structure
    time_t now_c = system_clock::to_time_t(now);

    // Use localtime_s for secure conversion
    tm now_tm;
    localtime_s(&now_tm, &now_c);

    // Extract milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Format the string
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d-%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string getTime(std::chrono::system_clock::time_point now)
{
    using namespace std::chrono;

    // Convert to time_t for conversion to tm structure
    time_t now_c = system_clock::to_time_t(now);

    // Use localtime_s for secure conversion
    tm now_tm;
    localtime_s(&now_tm, &now_c);

    // Extract milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Format the string
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d-%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}


enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    FINAL
};

// log messages with different levels (colors)
void log(LogLevel level, const string& message) {
    string levelStr;
    string color;
    switch (level) {
        case LogLevel::INFO:
            levelStr = "INFO";
            color = GREEN;
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            color = YELLOW;
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            color = RED;
            break;
        case LogLevel::FINAL:
            levelStr = "FINAL";
            color = BLUE;
            break;
    }
    cout << color << getTime() << " [" << levelStr << "] " << message << RESET << endl;
}

// get Product object from identifier
// Define a type for a function that takes no arguments and returns a T
template <typename T>
using ProductCtor = std::function<T()>;

// Define a map from CUSIPs to product constructors
template <typename T>
std::map<string, ProductCtor<T>> productConstructors = {
    {"9128283H1", []() { return Bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30")); }},
    {"9128283L2", []() { return Bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15")); }},
    {"912828M80", []() { return Bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30")); }},
    {"9128283J7", []() { return Bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30")); }},
    {"9128283F5", []() { return Bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15")); }},
    {"912810TW8", []() { return Bond("912810TW8", CUSIP, "US20Y", 0.02500, from_string("2037/12/15")); }},
    {"912810RZ3", []() { return Bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15")); }},
};

template <typename T>
T QueryProduct(const string& cusip)
{
    auto it = productConstructors<T>.find(cusip);
    if (it == productConstructors<T>.end()) {
        throw std::invalid_argument("Unknown CUSIP: " + cusip);
    }
    return it->second();
}

// Function to calculate PV01
double CalculatePV01(double faceValue, double couponRate, double yieldRate, int yearsToMaturity, int frequency)
{
    double coupon = faceValue * couponRate / frequency;
    double pv01Initial = 0.0;

    for (int t = 1; t <= yearsToMaturity * frequency; ++t)
    {
        double discountFactor = 1.0 / pow(1.0 + yieldRate / frequency, t);
        pv01Initial += coupon * discountFactor;
    }
    pv01Initial += faceValue / pow(1.0 + yieldRate / frequency, yearsToMaturity * frequency);

    double yieldRateIncreased = yieldRate + 0.0001;
    double pv01Adjusted = 0.0;

    for (int t = 1; t <= yearsToMaturity * frequency; ++t)
    {
        double discountFactor = 1.0 / pow(1.0 + yieldRateIncreased / frequency, t);
        pv01Adjusted += coupon * discountFactor;
    }
    pv01Adjusted += faceValue / pow(1.0 + yieldRateIncreased / frequency, yearsToMaturity * frequency);

    return pv01Initial - pv01Adjusted;
}


// Define a map from CUSIPs to PV01
// Current yield for 2,3,5,7,10,20,30 year US treasury bonds: 0.0464, 0.0440, 0.0412, 0.043, 0.0428, 0.0461, 0.0443
std::map<string, double> pv01 = {
    {"9128283H1", CalculatePV01(1000, 0.01750, 0.0464, 2, 2)},
    {"9128283L2", CalculatePV01(1000, 0.01875, 0.0440, 3, 2)},
    {"912828M80", CalculatePV01(1000, 0.02000, 0.0412, 5, 2)},
    {"9128283J7", CalculatePV01(1000, 0.02125, 0.0430, 7, 2)},
    {"9128283F5", CalculatePV01(1000, 0.02250, 0.0428, 10, 2)},
    {"912810TW8", CalculatePV01(1000, 0.02500, 0.0461, 20, 2)},
    {"912810RZ3", CalculatePV01(1000, 0.02750, 0.0443, 30, 2)},
};

// Get unit PV01 value from CUSIP
double QueryPV01(const string& cusip) {
    auto it = pv01.find(cusip);
    if (it == pv01.end()) {
        throw std::invalid_argument("Unknown CUSIP: " + cusip);
    }
    return it->second;
}

double Frac2Price(const string& PriceFrac)
{
    int posDash = PriceFrac.find('-');
    if (posDash == string::npos) {
        throw std::invalid_argument("Invalid format: Dash '-' not found.");
    }

    double price1 = stod(PriceFrac.substr(0, posDash));

    string fractionalPart = PriceFrac.substr(posDash + 1);
    if (fractionalPart.size() != 3) {
        throw std::invalid_argument("Invalid format: Fractional part should be 3 digits.");
    }

    // Replace '+' with '4' if present in the z position
    if (fractionalPart[2] == '+') {
        fractionalPart[2] = '4';
    }

    double price32 = stod(fractionalPart.substr(0, 2)) / 32.0;
    double price256 = stod(fractionalPart.substr(2, 1)) / 256.0;

    return price1 + price32 + price256;
}

// Convert prices from decimal notation to fractional notation.
string Price2Frac(double price)
{
    int intpart = static_cast<int>(floor(price));
    double fracpart = price - intpart;

    int xy = static_cast<int>(fracpart * 32);
    int z = static_cast<int>(fracpart * 256) % 8;

    string fractionalString = to_string(intpart) + "-";
    fractionalString += (xy < 10 ? "0" : "") + to_string(xy);
    fractionalString += (z == 4 ? "+" : to_string(z));

    return fractionalString;
}

// generate oscillating spread between 1/64 and 1/128
double genRandomSpread(std::mt19937& gen) {
    std::uniform_real_distribution<double> dist(1.0/128.0, 1.0/64.0);
    return dist(gen);
}

// Generate random ID with numbers and letters
string GenerateRandomId(long length)
{
    string id = "";
    for (int j = 0; j < length; ++j) {
        int random = rand() % 36;
        if (random < 10) {
            id += to_string(random);
        } else {
            id += static_cast<char>('A' + random - 10);
        }
    }
    return id;
}

/**
 * 1. Generate prices that oscillate between 99 and 101 and write to prices.txt
 * 2. Generate order book data with fivel levels of bids and offers and write to marketdata.txt
 */
void genOrderBook(const vector<string>& products, const string& priceFile, const string& orderbookFile, long long seed, const int numDataPoints) {
    std::ofstream pFile(priceFile);
    std::ofstream oFile(orderbookFile);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> ms_dist(1, 20); // simulate milliseconds increments

    // price file format: Timestamp, CUSIP, Bid, Ask
    pFile << "Timestamp,CUSIP,Bid,Ask" << endl;

    // orderbook file format: Timestamp, CUSIP, Bid1, BidSize1, Ask1, AskSize1, Bid2, BidSize2, Ask2, AskSize2, Bid3, BidSize3, Ask3, AskSize3, Bid4, BidSize4, Ask4, AskSize4, Bid5, BidSize5, Ask5, AskSize5
    oFile << "Timestamp,CUSIP,Bid1,BidSize1,Ask1,AskSize1,Bid2,BidSize2,Ask2,AskSize2,Bid3,BidSize3,Ask3,AskSize3,Bid4,BidSize4,Ask4,AskSize4,Bid5,BidSize5,Ask5,AskSize5" << endl;

    for (const auto& product : products) {
        double midPrice = 99.00;
        bool priceIncreasing = true;
        bool spreadIncreasing = true;
        double fixSpread = 1.0/128.0;
        auto curTime = std::chrono::system_clock::now();
        string timestamp;

        // number of data points
        for (int i = 0; i < numDataPoints; ++i) {

            // generate price data
            double randomSpread = genRandomSpread(gen);
            curTime += std::chrono::milliseconds(ms_dist(gen));
            timestamp = getTime(curTime);

            double randomBid = midPrice - randomSpread / 2.0;
            double randomAsk = midPrice + randomSpread / 2.0;
            pFile << timestamp << "," << product << "," << Price2Frac(randomBid) << "," << Price2Frac(randomAsk) << "," << randomSpread << endl;

            // generate order book data
            oFile << timestamp << "," << product;
            for (int level=1; level<=5; ++level){
                double fixBid = midPrice - fixSpread * level / 2.0;
                double fixAsk = midPrice + fixSpread * level / 2.0;
                int size = level * 1'000'000;
                oFile << "," << Price2Frac(fixBid) << "," << size << "," << Price2Frac(fixAsk) << "," << size;
            }
            oFile << endl;

            // oscillate mid price
            if (priceIncreasing) {
                midPrice += 1.0 / 256.0;
                if (randomAsk >= 101.0) {
                    priceIncreasing = false;
                }
            } else {
                midPrice -= 1.0 / 256.0;
                if (randomBid <= 99.0) {
                    priceIncreasing = true;
                }
            }

            // oscillate spread
            if (spreadIncreasing) {
                fixSpread += 1.0 / 128.0;
                if (fixSpread >= 1.0 / 32.0) {
                    spreadIncreasing = false;
                }
            } else {
                fixSpread -= 1.0 / 128.0;
                if (fixSpread <= 1.0 / 128.0) {
                    spreadIncreasing = true;
                }
            }
        }
    }

    pFile.close();
    oFile.close();
}

/**
 * Generate trades data
 */
void genTrades(const vector<string>& products, const string& tradeFile, long long seed) {
    vector<string> books = {"TRSY1", "TRSY2", "TRSY3"};
    vector<long> quantities = {1000000, 2000000, 3000000, 4000000, 5000000};
    std::ofstream tFile(tradeFile);
    std::mt19937 gen(seed);

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random trade id with number and letters
            string tradeId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string book = books[i % books.size()];

        tFile << product << "," << tradeId << "," << Price2Frac(price) << "," << book << "," << quantity << "," << side << endl;
        }
    }

    tFile.close();
}

/**
 * Generate inquiry data
 */
void genInquiries(const vector<string>& products, const string& inquiryFile, long long seed){
    std::ofstream iFile(inquiryFile);
    std::mt19937 gen(seed);
    vector<long> quantities = {1000000, 2000000, 3000000, 4000000, 5000000};

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random inquiry id with number and letters
            string inquiryId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string status = "RECEIVED";

        iFile << inquiryId << "," << product << "," << side << "," << quantity << "," << Price2Frac(price) << "," << status << endl;
        }
    }
}



#endif
