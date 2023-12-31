// tradebookingservice.hpp
//
// Purpose: 1.  Defines the data types and Service for trade booking.
// 
// @author Breman Thuraisingham
// @coauthor Yuanting Li
// @version 2.0 2023/12/23

#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "executionservice.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

    // default ctor
    Trade() = default;

    // ctor for a trade
    Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side);

    // dtor
    ~Trade() = default;

    // Get the product
    const T& GetProduct() const;

    // Get the trade ID
    const string& GetTradeId() const;

    // Get the mid price
    double GetPrice() const;

    // Get the book
    const string& GetBook() const;

    // Get the quantity
    long GetQuantity() const;

    // Get the side
    Side GetSide() const;

private:
    T product;
    string tradeId;
    double price;
    string book;
    long quantity;
    Side side;

};


template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
    tradeId = _tradeId;
    price = _price;
    book = _book;
    quantity = _quantity;
    side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
    return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
    return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
    return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
    return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
    return side;
}

// fwd declaration
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingServiceListener;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
  map<string, Trade<T>> tradeData; // store trade data keyed by trade id
  vector<ServiceListener<Trade<T>>*> listeners; // list of listeners to this service
  TradeBookingConnector<T>* connector;
  TradeBookingServiceListener<T>* tradebookinglistener;

public:
  // ctor and dtor
  TradeBookingService();
  ~TradeBookingService()=default;

  // Get data
  Trade<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Trade<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<Trade<T>> *listener);

  // Get all listeners on the Service.
  const vector< ServiceListener<Trade<T>>* >& GetListeners() const;

  // Get the connector
  TradeBookingConnector<T>* GetConnector();

  // Get associated trade book listener
  TradeBookingServiceListener<T>* GetTradeBookingServiceListener();

  // Book the trade
  void BookTrade(Trade<T> &trade);

};

template<typename T>
TradeBookingService<T>::TradeBookingService()
{
  connector = new TradeBookingConnector<T>(this);
  tradebookinglistener = new TradeBookingServiceListener<T>(this);
}

template<typename T>
Trade<T>& TradeBookingService<T>::GetData(string key)
{
    auto it = tradeData.find(key);
    if (it != tradeData.end())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("Key not found");
    }
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T> &data)
{
  string key = data.GetTradeId();
  if (tradeData.find(key) != tradeData.end())
    tradeData[key] = data;
  else
    tradeData.insert(pair<string, Trade<T>>(key, data));

  for(auto& listener : listeners)
    listener->ProcessAdd(data);
}

template<typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Trade<T>>* >& TradeBookingService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
TradeBookingConnector<T>* TradeBookingService<T>::GetConnector()
{
  return connector;
}

template<typename T>
TradeBookingServiceListener<T>* TradeBookingService<T>::GetTradeBookingServiceListener()
{
  return tradebookinglistener;
}


/**
 * Book a trade and send the information to the listener
 */
template<typename T>
void TradeBookingService<T>::BookTrade(Trade<T> &trade)
{
    for(auto& l : listeners)
        l->ProcessAdd(trade);

}

/**
* Connector that subscribes data from socket to trade booking service.
* Type T is the product type.
*/
template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{
private:
  TradeBookingService<T>* service;

public:
  // ctor
  TradeBookingConnector(TradeBookingService<T>* _service);
  // dtor
  ~TradeBookingConnector()=default;

  // Publish data to the Connector
  void Publish(Trade<T> &data) override;

  // Subscribe data from the Connector
  void Subscribe(ifstream& _data);

};

template<typename T>
TradeBookingConnector<T>::TradeBookingConnector(TradeBookingService<T>* _service)
{
  service = _service;
}

template<typename T>
void TradeBookingConnector<T>::Publish(Trade<T> &data)
{
}

template<typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& _data)
{
    string line;
    while (getline(_data, line))
    {
        stringstream lineStream(line);
        vector<string> tokens;
        string token;

        while (getline(lineStream, token, ','))
        {
            tokens.push_back(token);
        }

        string productId = tokens[0];
        T product = QueryProduct<T>(productId);
        string tradeId = tokens[1];
        double price = Frac2Price(tokens[2]);
        string book = tokens[3];
        long quantity = stol(tokens[4]);
        Side side = (tokens[5] == "BUY") ? BUY : SELL;

        Trade<T> trade(product, tradeId, price, book, quantity, side);
        service->OnMessage(trade);
    }
}


/**
 * Trade Booking Execution Listener subscribing from execution service.
 * Basically, this listener is used to subscribe data from execution service,
 * transfer the ExecutionOrder<T> data to Trade<T> data, and call BookTrade()
 * method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
class TradeBookingServiceListener : public ServiceListener<ExecutionOrder<T>>
{
private:
  TradeBookingService<T>* service;
  long count;

public:
  // ctor
  TradeBookingServiceListener(TradeBookingService<T>* _service);
  // dtor
  ~TradeBookingServiceListener()=default;

  // Listener callback to process an add event to the Service
  void ProcessAdd(ExecutionOrder<T> &data) override;

  // Listener callback to process a remove event to the Service
  void ProcessRemove(ExecutionOrder<T> &data) override;

  // Listener callback to process an update event to the Service
  void ProcessUpdate(ExecutionOrder<T> &data) override;

};

template<typename T>
TradeBookingServiceListener<T>::TradeBookingServiceListener(TradeBookingService<T>* _service)
{
  service = _service;
  count = 0;
}


/**
 * ProcessAdd() method is used to transfer ExecutionOrder<T> data to Trade<T> data,
 * and then call BookTrade() method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
void TradeBookingServiceListener<T>::ProcessAdd(ExecutionOrder<T>& order)
{
    T product = order.GetProduct();
    string orderId = order.GetOrderId();
    double price = order.GetPrice();
    long visibleQuantity = order.GetVisibleQuantity();
    long hiddenQuantity = order.GetHiddenQuantity();
    long totalQuantity = visibleQuantity + hiddenQuantity;

    Side tradeSide = (order.GetSide() == BID) ? BUY : SELL;

    string book;
    switch (count++ % 3)
    {
    case 0: book = "TRSY1"; break;
    case 1: book = "TRSY2"; break;
    case 2: book = "TRSY3"; break;
    }

    Trade<T> trade(product, orderId, price, book, totalQuantity, tradeSide);
    service->BookTrade(trade);
}

template<typename T>
void TradeBookingServiceListener<T>::ProcessRemove(ExecutionOrder<T> &data)
{
}

template<typename T>
void TradeBookingServiceListener<T>::ProcessUpdate(ExecutionOrder<T> &data)
{
}


#endif






