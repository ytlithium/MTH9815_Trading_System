 // inquiryservice.hpp
 //
 // Purpose: 1.  Defines the data types and Servicecustomer inquiries.
 // 
 // @author Breman Thuraisingham
 // @coauthor Yuanting Li
 // @version 2.0 2023/12/24

#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "utilities.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

    // default ctor
    Inquiry() = default;

    // ctor for an inquiry
    Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state);

    // dtor
    ~Inquiry() = default;

    // Get the inquiry ID
    const string& GetInquiryId() const;

    // Get the product
    const T& GetProduct() const;

    // Get the side on the inquiry
    Side GetSide() const;

    // Get the quantity that the client is inquiring for
    long GetQuantity() const;

    // Get the price that we have responded back with
    double GetPrice() const;

    // Get the current state on the inquiry
    InquiryState GetState() const;

    // Set the price
    void SetPrice(double _price);

    // Set the current state on the inquiry
    void SetState(InquiryState state);

    // object printer
    template<typename S>
    friend ostream& operator<<(ostream& output, const Inquiry<S>& inquiry);

private:
    string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;

};


template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state) :
    product(_product), inquiryId(_inquiryId), side(_side), quantity(_quantity), price(_price), state(_state)
{
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
    return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
    return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
    return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
    return price;
}

template<typename T>
void Inquiry<T>::SetPrice(double _price)
{
    price = _price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
    return state;
}

template<typename T>
void Inquiry<T>::SetState(InquiryState _state)
{
    state = _state;
}

template<typename T>
ostream& operator<<(ostream& output, const Inquiry<T>& inquiry)
{
    string inquiryId = inquiry.GetInquiryId();
    T product = inquiry.GetProduct();
    string productId = product.GetProductId();

    Side side = inquiry.GetSide();
    string _side;
    switch (side)
    {
     case BID:
      _side = "BID";
      break;
     case OFFER:
     _side = "OFFER";
      break;
    }

    long quantity = inquiry.GetQuantity();
    double price = inquiry.GetPrice();

    string state;
    switch (inquiry.GetState())
    {
    case RECEIVED:
        state = "RECEIVED";
        break;
    case QUOTED:
        state = "QUOTED";
        break;
    case DONE:
        state = "DONE";
        break;
    case REJECTED:
        state = "REJECTED";
        break;
    case CUSTOMER_REJECTED:
        state = "CUSTOMER_REJECTED";
        break;
    }

    output << inquiryId << "," << productId << "," << _side << ","
        << quantity << "," << Price2Frac(price) << "," << state;

    return output;
}

/**
 * Forward declaration for InquiryConnector
 */
template<typename T>
class InquiryConnector;


/**
 * Service for customer inquiry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
private:
    InquiryConnector<T>* connector;
    map<string, Inquiry<T>> inquiryData;
    vector<ServiceListener<Inquiry<T>>*> listeners;

public:
    // ctor and dtor
    InquiryService();
    ~InquiryService()=default;

    // Get data on our service given a key
    Inquiry<T>& GetData(string key);

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Inquiry<T> &data);

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service.
    void AddListener(ServiceListener<Inquiry<T>> *listener);

    // Get all listeners on the Service.
    const vector< ServiceListener<Inquiry<T>>* >& GetListeners() const;

    // Get the connector
    InquiryConnector<T>* GetConnector();

    // Send a quote back to the client
    void SendQuote(const string &inquiryId, double price);

    // Reject an inquiry from the client
    void RejectInquiry(const string &inquiryId);

};

template<typename T>
InquiryService<T>::InquiryService() : connector(new InquiryConnector<T>(this))
{
}

template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string key)
{
    auto it = inquiryData.find(key);
    if (it != inquiryData.end())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("Key not found");
    }
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& data)
{
    InquiryState state = data.GetState();
    string inquiryId = data.GetInquiryId();
    switch (state) {
    case RECEIVED:
        // if inquiry is received, send back a quote to the connector via publish()
        connector->Publish(data);
        break;
    case QUOTED:
        // finish the inquiry with DONE status and send an update of the object
        data.SetState(DONE);
        // store the inquiry
        if (inquiryData.find(inquiryId) != inquiryData.end()) { inquiryData.erase(inquiryId); }
        inquiryData.insert(pair<string, Inquiry<T> >(inquiryId, data));
        // notify listeners
        for (auto& listener : listeners)
        {
            listener->ProcessAdd(data);
        }
        break;
    default:
        break;
    }


    // if inquiry is done, remove it from the map
    if (data.GetState() == DONE)
    {
        inquiryData.erase(data.GetInquiryId());
    }
    // otherwise, update the inquiry
    else
    {
        inquiryData[data.GetInquiryId()] = data;
    }

    // notify listeners
    for (auto& listener : listeners)
    {
        listener->ProcessAdd(data);
    }
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Inquiry<T>>* >& InquiryService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
InquiryConnector<T>* InquiryService<T>::GetConnector()
{
  return connector;
}

template<typename T>
void InquiryService<T>::SendQuote(const string &inquiryId, double price)
{
    // get the inquiry
    Inquiry<T>& inquiry = inquiryData[inquiryId];
    // update the inquiry
    inquiry.SetPrice(price);
    // notify listeners
    for (auto& listener : listeners)
    {
    listener->ProcessAdd(inquiry);
    }
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string &inquiryId)
{
    // get the inquiry
    Inquiry<T>& inquiry = inquiryData[inquiryId];
    // update the inquiry
    inquiry.SetState(REJECTED);
}

/**
* Inquiry Connector subscribing data to Inquiry Service and publishing data to Inquiry Service.
* Type T is the product type.
*/
template <typename T>
class InquiryConnector : public Connector<Inquiry<T>>
{
private:
    InquiryService<T>* service;

public:
    // ctor
    InquiryConnector(InquiryService<T>* _service);
    // dtor
    ~InquiryConnector()=default;

    // Publish data to the Connector
    // If subscribe-only, then this does nothing
    void Publish(Inquiry<T> &data) override;

    // Subscribe data from connector
    void Subscribe(ifstream& _datafile);

    // Subcribe updated inquiry record from the connector
    void SubscribeUpdate(Inquiry<T>& data);
};

template<typename T>
InquiryConnector<T>::InquiryConnector(InquiryService<T>* _service)
: service(_service)
{
}

// Transite the inquiry from RECEIVED to QUOTED and send back to the service
template <typename T>
void InquiryConnector<T>::Publish(Inquiry<T> &data)
{
    if (data.GetState() == RECEIVED){
    data.SetState(QUOTED);
    this->SubscribeUpdate(data);
    }
}

template <typename T>
void InquiryConnector<T>::Subscribe(ifstream& _datafile)
{
    string line;
    while (getline(_datafile, line))
    {
        // parse the line
        vector<string> tokens;
        stringstream ss(line);
        string token;
        while (getline(ss, token, ',')) 
        {
            tokens.push_back(token);
        }
        // create inquiry
        string inquiryId = tokens[0];
        string productId = tokens[1];
        T product = QueryProduct<T>(productId);
        Side side = tokens[2] == "BUY" ? BUY : SELL;
        long quantity = stol(tokens[3]);
        double price = Frac2Price(tokens[4]);
        InquiryState state = tokens[5] == "RECEIVED" ? RECEIVED : tokens[5] == "QUOTED" ? QUOTED : tokens[5] == "DONE" ? DONE : tokens[5] == "REJECTED" ? REJECTED : CUSTOMER_REJECTED;
        Inquiry<T> inquiry(inquiryId, product, side, quantity, price, state);
        service->OnMessage(inquiry);
    }
}

template<typename T>
void InquiryConnector<T>::SubscribeUpdate(Inquiry<T>& data)
{
    // send updated inquiry back to the service
    service->OnMessage(data);
}


#endif
