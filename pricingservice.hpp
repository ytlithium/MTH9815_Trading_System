// pricingservice.hpp
//
// Purpose: 1. Defines the data types and Service for internal prices.
// 2. Design PricingService and PricingConnector for price data query and updating.
// 
// @author Breman Thuraisingham
// @coauthor Yuanting Li
// @version 2.0 2023/12/22 

#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include <map>
#include <fstream>
#include "soa.hpp"
#include "utilities.hpp"

 /**
  * A price object consisting of mid and bid/offer spread.
  * Type T is the product type.
  */
template<typename T>
class Price
{

public:
    // default ctor (needed for map data structure later)
    Price() = default;

    // ctor for a price
    Price(const T& _product, double _mid, double _bidOfferSpread);

    // dtor
    ~Price() = default;

    // Get the product
    const T& GetProduct() const;

    // Get the mid price
    double GetMid() const;

    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;

    // Print the price object
    template<typename S>
    friend ostream& operator<<(ostream& output, const Price<S>& bond);

private:
    T product;
    double mid;
    double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread)
    : product(_product), mid(_mid), bidOfferSpread(_bidOfferSpread)
{
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}

// Print the price object
template<typename T>
ostream& operator<<(ostream& output, const Price<T>& price)
{
    output << price.GetProduct().GetProductId() << " Mid: " << price.GetMid()
        << ", Spread: " << price.GetBidOfferSpread();
    return output;
}

// forward declaration of PricingConnector
template<typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string, Price<T>>
{
private:
    map<string, Price<T>> priceData; // store price data keyed by product identifier
    vector<ServiceListener<Price<T>>*> listeners; // list of listeners to this service
    PricingConnector<T>* connector; // connector related to this server

public:
    // ctor
    PricingService();
    // dtor
    ~PricingService() = default;


    // Get data on our service given a key
    Price<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Price<T>>* listener) override;

    // Get all listeners on the Service.
    const vector< ServiceListener<Price<T>>* >& GetListeners() const override;

    // Get the connector
    PricingConnector<T>* GetConnector();

};

template<typename T>
PricingService<T>::PricingService() : connector(new PricingConnector<T>(this))
{
}


template<typename T>
Price<T>& PricingService<T>::GetData(string key)
{
    auto it = priceData.find(key);
    if (it != priceData.end())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("Key not found");
    }
}

template<typename T>
void PricingService<T>::OnMessage(Price<T>& data)
{
    // flow data
    string key = data.GetProduct().GetProductId();
    // update the price map
    if (priceData.find(key) != priceData.end()) 
    { 
        priceData.erase(key); 
    }
    priceData.insert(pair<string, Price<Bond> >(key, data));

    // flow the data to listeners
    for (auto& l : listeners) {
        l->ProcessAdd(data);
    }
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* listener)
{
    listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Price<T>>* >& PricingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
PricingConnector<T>* PricingService<T>::GetConnector()
{
    return connector;
}


/**
 * PricingConnector: an inbound connector that subscribes data from socket to pricing service.
 * Type T is the product type.
 */
template<typename T>
class PricingConnector : public Connector<Price<T>>
{
private:
    PricingService<T>* service;

public:
    // ctor

    PricingConnector(PricingService<T>* _service);
    // dtor
    ~PricingConnector() = default;

    // Publish data to the Connector
    // If subscribe-only, this does nothing
    void Publish(Price<T>& data) override;

    // Subscribe data
    void Subscribe(ifstream& _data);

};

template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* _service)
    : service(_service)
{
}

// inbound connector, does nothing
template <typename T>
void PricingConnector<T>::Publish(Price<T>& data)
{
}

template<typename T>
void PricingConnector<T>::Subscribe(ifstream& _data)
{
    string line;
    // Skip the header
    getline(_data, line); 

    // read lines in the data 
    while (getline(_data, line))
    {
        stringstream rawline(line);
        string block;
        vector<string> splitdata;

        // from the raw text split data into blocks
        while (getline(rawline, block, ','))
        {
            splitdata.push_back(block);
        }

        string timestamp = splitdata[0];
        string productID = splitdata[1];

        // Convert the raw
        double bid = Frac2Price(splitdata[2]);
        double ask = Frac2Price(splitdata[3]);

        // Calculate mid and spread
        double mid = (bid + ask) / 2.0;
        double spread = ask - bid;

        // Get the product
        T product = QueryProduct<T>(productID);
        Price<T> price(product, mid, spread);

        // Update by communication
        service->OnMessage(price);
    }
}

#endif